#include "vader5/hidraw.hpp"
#include "vader5/protocol.hpp"

#ifdef VADER5_USE_HIDAPI

#include <hidapi.h>

#include <climits>
#include <cstring>
#include <utility>

namespace vader5 {

constexpr uint8_t DPAD_MASK = 0x0F;

auto find_hidraw_device(uint16_t vid, uint16_t pid, int iface_num) -> Result<std::string> {
    // hidapi-libusb returns paths like "0003:37D7:2401.0003" for interface 3
    struct hid_device_info* devs = hid_enumerate(vid, pid);
    struct hid_device_info* cur = devs;
    std::string result_path;
    while (cur) {
        if (iface_num < 0 || cur->interface_number == iface_num) {
            result_path = cur->path;
            break;
        }
        cur = cur->next;
    }
    hid_free_enumeration(devs);

    if (result_path.empty()) {
        return std::unexpected(std::make_error_code(std::errc::no_such_device));
    }
    return result_path;
}

auto Hidraw::open(uint16_t vid, uint16_t pid, int iface,
                  const std::string& device_name) -> Result<Hidraw> {
    if (hid_init() != 0) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }

    hid_device* device = nullptr;

    if (!device_name.empty()) {
        // Open by path (e.g., "hidraw0")
        device = hid_open_path(device_name.c_str());
    } else {
        // Try hid_open first (works when kernel driver is not attached)
        device = hid_open(vid, pid, nullptr);
        if (!device) {
            // hid_open failed (kernel driver attached). Fallback to path-based open.
            struct hid_device_info* devs = hid_enumerate(vid, pid);
            struct hid_device_info* cur = devs;
            std::string target_path;
            while (cur) {
                if (iface < 0 || cur->interface_number == iface) {
                    target_path = cur->path;
                    break;
                }
                cur = cur->next;
            }
            hid_free_enumeration(devs);
            if (!target_path.empty()) {
                device = hid_open_path(target_path.c_str());
            }
        } else if (iface >= 0) {
            // hid_open succeeded but wrong interface; reopen by path
            struct hid_device_info* devs = hid_enumerate(vid, pid);
            struct hid_device_info* cur = devs;
            std::string target_path;
            while (cur) {
                if (cur->interface_number == iface) {
                    target_path = cur->path;
                    break;
                }
                cur = cur->next;
            }
            hid_free_enumeration(devs);
            if (!target_path.empty()) {
                hid_close(device);
                device = hid_open_path(target_path.c_str());
            }
        }
    }

    if (!device) {
        hid_exit();
        return std::unexpected(std::make_error_code(std::errc::no_such_device));
    }

    // Set non-blocking mode for hid_read_timeout
    hid_set_nonblocking(device, 1);

    return Hidraw(device);
}

Hidraw::~Hidraw() {
    if (device_) {
        hid_close(device_);
        hid_exit();
    }
}

auto Hidraw::reconnect(uint16_t vid, uint16_t pid, int iface) -> Result<void> {
    // Close existing device
    if (device_) {
        hid_close(device_);
        device_ = nullptr;
    }
    // Re-open
    hid_device* device = hid_open(vid, pid, nullptr);
    if (!device) {
        struct hid_device_info* devs = hid_enumerate(vid, pid);
        struct hid_device_info* cur = devs;
        std::string target_path;
        while (cur) {
            if (iface < 0 || cur->interface_number == iface) {
                target_path = cur->path;
                break;
            }
            cur = cur->next;
        }
        hid_free_enumeration(devs);
        if (!target_path.empty()) {
            device = hid_open_path(target_path.c_str());
        }
    }
    if (!device) {
        return std::unexpected(std::make_error_code(std::errc::no_such_device));
    }
    hid_set_nonblocking(device, 1);
    device_ = device;
    return {};
}

Hidraw::Hidraw(Hidraw&& other) noexcept : device_(other.device_) {
    other.device_ = nullptr;
}

auto Hidraw::operator=(Hidraw&& other) noexcept -> Hidraw& {
    if (this != &other) {
        if (device_) {
            hid_close(device_);
            hid_exit();
        }
        device_ = other.device_;
        other.device_ = nullptr;
    }
    return *this;
}

auto Hidraw::phys() const -> Result<std::string> {
    // hidapi does not expose the physical device path (e.g. "usb-0000:00:14.0-4/input2").
    // This means block_redundant_input() in gamepad.cpp will gracefully skip the
    // redundant input device suppression.
    return std::unexpected(std::make_error_code(std::errc::function_not_supported));
}

auto Hidraw::read(std::span<uint8_t> buf) const -> Result<size_t> {
    if (!device_) {
        return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
    }
    int bytes = hid_read_timeout(device_, buf.data(), buf.size(), 10);
    if (bytes < 0) {
        return std::unexpected(std::make_error_code(std::errc::resource_unavailable_try_again));
    }
    return static_cast<size_t>(bytes);
}

auto Hidraw::write(std::span<const uint8_t> buf) const -> Result<size_t> {
    if (!device_) {
        return std::unexpected(std::make_error_code(std::errc::bad_file_descriptor));
    }
    int bytes = hid_write(device_, buf.data(), buf.size());
    if (bytes < 0) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
    return static_cast<size_t>(bytes);
}

// parse_report and parse_report_24g are backend-independent (pure data parsing).
// They are included here to keep them with the Hidraw class.

auto Hidraw::parse_report(std::span<const uint8_t> data) -> std::optional<GamepadState> {
    constexpr size_t REPORT_24G = 20;
    constexpr uint8_t SUBTYPE_24G = 0x14;

    if (data.size() == REPORT_24G && data[1] == SUBTYPE_24G) {
        return parse_report_24g(data);
    }
    return std::nullopt;
}

auto Hidraw::parse_report_24g(std::span<const uint8_t> data) -> std::optional<GamepadState> {
    constexpr size_t OFF_MISC = 2;
    constexpr size_t OFF_BTNS = 3;
    constexpr size_t OFF_LT = 4;
    constexpr size_t OFF_RT = 5;
    constexpr size_t OFF_LX = 6;
    constexpr size_t OFF_LY = 8;
    constexpr size_t OFF_RX = 10;
    constexpr size_t OFF_RY = 12;
    constexpr size_t OFF_EXT1 = 14;
    constexpr size_t OFF_EXT2 = 15;

    GamepadState state{};
    const uint8_t misc = data[OFF_MISC];
    const uint8_t btns = data[OFF_BTNS];
    state.dpad = DPAD_MAP[misc & DPAD_MASK];
    state.buttons = static_cast<uint16_t>(
        (((misc >> 4) & 1) * PAD_START) | (((misc >> 5) & 1) * PAD_SELECT) |
        (((misc >> 6) & 1) * PAD_L3) | (((misc >> 7) & 1) * PAD_R3) | (((btns >> 0) & 1) * PAD_LB) |
        (((btns >> 1) & 1) * PAD_RB) | (((btns >> 3) & 1) * PAD_MODE) |
        (((btns >> 4) & 1) * PAD_A) | (((btns >> 5) & 1) * PAD_B) | (((btns >> 6) & 1) * PAD_X) |
        (((btns >> 7) & 1) * PAD_Y));
    state.left_trigger = data[OFF_LT];
    state.right_trigger = data[OFF_RT];
    state.left_x = read_s16(&data[OFF_LX]);
    state.left_y = static_cast<int16_t>(-read_s16(&data[OFF_LY]));
    state.right_x = read_s16(&data[OFF_RX]);
    state.right_y = static_cast<int16_t>(-read_s16(&data[OFF_RY]));
    state.ext_buttons = data[OFF_EXT1];
    state.ext_buttons2 = data[OFF_EXT2];
    return state;
}

} // namespace vader5

#else
// When not using hidapi, include the original Linux hidraw implementation.
// This file is handled by src/hidraw.cpp in the default build.
#endif // VADER5_USE_HIDAPI
