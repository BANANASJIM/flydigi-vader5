#include "vader5/hidraw.hpp"

#include <fcntl.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <string_view>

namespace vader5 {
namespace fs = std::filesystem;

namespace {
constexpr int HID_ID_PREFIX_LEN = 1;
constexpr int HID_ID_FIELD_LEN = 8;
constexpr int HID_ID_PRODUCT_OFFSET = 10;
} // namespace

auto find_hidraw_device(uint16_t vid, uint16_t pid, int iface_num) -> Result<std::string> {
    for (const auto& entry : fs::directory_iterator("/sys/class/hidraw")) {
        const auto device_path = entry.path() / "device";
        if (!fs::exists(device_path)) {
            continue;
        }

        const auto uevent_path = device_path / "uevent";
        std::ifstream uevent(uevent_path);
        if (!uevent) {
            continue;
        }

        std::string line;
        bool vid_match = false;
        bool pid_match = false;
        bool iface_match = (iface_num < 0);

        while (std::getline(uevent, line)) {
            if (line.starts_with("HID_ID=")) {
                const auto pos = line.find(':');
                if (pos != std::string::npos) {
                    try {
                        const auto vendor_str =
                            line.substr(pos + HID_ID_PREFIX_LEN, HID_ID_FIELD_LEN);
                        const auto product_str =
                            line.substr(pos + HID_ID_PRODUCT_OFFSET, HID_ID_FIELD_LEN);
                        const auto parsed_vid = std::stoul(vendor_str, nullptr, 16);
                        const auto parsed_pid = std::stoul(product_str, nullptr, 16);
                        vid_match = (parsed_vid == vid);
                        pid_match = (parsed_pid == pid);
                    } catch (...) {
                        continue;
                    }
                }
            }
            if (iface_num >= 0 && line.starts_with("HID_PHYS=")) {
                constexpr std::string_view INPUT_PREFIX = "/input";
                const auto pos = line.rfind(INPUT_PREFIX);
                if (pos != std::string::npos) {
                    try {
                        const auto iface_str = line.substr(pos + INPUT_PREFIX.size());
                        iface_match = (std::stoi(iface_str) == iface_num);
                    } catch (...) {
                        continue;
                    }
                }
            }
        }

        if (vid_match && pid_match && iface_match) {
            return "/dev/" + entry.path().filename().string();
        }
    }
    return std::unexpected(std::make_error_code(std::errc::no_such_device));
}

auto Hidraw::open(uint16_t vid, uint16_t pid, int iface) -> Result<Hidraw> {
    auto dev_path = find_hidraw_device(vid, pid, iface);
    if (!dev_path) {
        return std::unexpected(dev_path.error());
    }

    const int file_descriptor = ::open(dev_path->c_str(), O_RDWR | O_NONBLOCK);
    if (file_descriptor < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    return Hidraw(file_descriptor);
}

Hidraw::~Hidraw() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

Hidraw::Hidraw(Hidraw&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

auto Hidraw::operator=(Hidraw&& other) noexcept -> Hidraw& {
    if (this != &other) {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

auto Hidraw::read(std::span<uint8_t> buf) const -> Result<size_t> {
    const auto bytes_read = ::read(fd_, buf.data(), buf.size());
    if (bytes_read < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }
    return static_cast<size_t>(bytes_read);
}

auto Hidraw::write(std::span<const uint8_t> buf) const -> Result<size_t> {
    const auto bytes_written = ::write(fd_, buf.data(), buf.size());
    if (bytes_written < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }
    return static_cast<size_t>(bytes_written);
}

auto Hidraw::parse_report(std::span<const uint8_t> data) -> std::optional<GamepadState> {
    constexpr size_t REPORT_SIZE_24G = 20;
    constexpr uint8_t SUBTYPE_24G = 0x14;

    if (data.size() == REPORT_SIZE_24G && data[1] == SUBTYPE_24G) {
        return parse_report_24g(data);
    }

    constexpr size_t MIN_REPORT_SIZE = 12;
    if (data.size() < MIN_REPORT_SIZE) {
        return std::nullopt;
    }

    GamepadState state;

    constexpr int AXIS_CENTER = 128;
    constexpr int AXIS_SCALE = 256;

    state.left_x = static_cast<int16_t>((data[1] - AXIS_CENTER) * AXIS_SCALE);
    state.left_y = static_cast<int16_t>((data[2] - AXIS_CENTER) * AXIS_SCALE);
    state.right_x = static_cast<int16_t>((data[3] - AXIS_CENTER) * AXIS_SCALE);
    state.right_y = static_cast<int16_t>((data[4] - AXIS_CENTER) * AXIS_SCALE);

    state.left_trigger = data[8];
    state.right_trigger = data[9];

    constexpr uint8_t DPAD_MASK = 0x0F;
    state.dpad = data[5] & DPAD_MASK;

    state.buttons = static_cast<uint16_t>(data[6]) | (static_cast<uint16_t>(data[7]) << 8);

    constexpr size_t EXT_BUTTON_OFFSET = 11;
    state.ext_buttons = data[EXT_BUTTON_OFFSET];

    return state;
}

auto Hidraw::parse_report_24g(std::span<const uint8_t> data) -> std::optional<GamepadState> {
    constexpr uint8_t DPAD_MASK = 0x0F;
    constexpr size_t OFF_MISC = 2;
    constexpr size_t OFF_BUTTONS = 3;
    constexpr size_t OFF_LT = 4;
    constexpr size_t OFF_RT = 5;
    constexpr size_t OFF_LX = 6;
    constexpr size_t OFF_LY = 8;
    constexpr size_t OFF_RX = 10;
    constexpr size_t OFF_RY = 12;

    GamepadState state{};
    const uint8_t misc = data[OFF_MISC];
    const uint8_t buttons = data[OFF_BUTTONS];
    const uint8_t dpad_bits = misc & DPAD_MASK;

    // Dpad bitmask lookup
    constexpr std::array<uint8_t, 16> DPAD_MAP = {
        DPAD_NONE, DPAD_UP, DPAD_DOWN, DPAD_NONE,
        DPAD_LEFT, DPAD_UP_LEFT, DPAD_DOWN_LEFT, DPAD_NONE,
        DPAD_RIGHT, DPAD_UP_RIGHT, DPAD_DOWN_RIGHT, DPAD_NONE,
        DPAD_NONE, DPAD_NONE, DPAD_NONE, DPAD_NONE
    };
    state.dpad = DPAD_MAP[dpad_bits];

    // Buttons from misc byte (Start/Select/L3/R3) and buttons byte
    state.buttons = static_cast<uint16_t>(
        (((misc >> 4) & 0x01) * BTN_START) |
        (((misc >> 5) & 0x01) * BTN_SELECT) |
        (((misc >> 6) & 0x01) * BTN_L3) |
        (((misc >> 7) & 0x01) * BTN_R3) |
        (((buttons >> 0) & 0x01) * BTN_LB) |
        (((buttons >> 1) & 0x01) * BTN_RB) |
        (((buttons >> 2) & 0x01) * BTN_MODE) |
        (((buttons >> 4) & 0x01) * BTN_A) |
        (((buttons >> 5) & 0x01) * BTN_B) |
        (((buttons >> 6) & 0x01) * BTN_X) |
        (((buttons >> 7) & 0x01) * BTN_Y)
    );

    state.left_trigger = data[OFF_LT];
    state.right_trigger = data[OFF_RT];

    auto read_s16 = [&data](size_t off) -> int16_t {
        return static_cast<int16_t>(
            static_cast<uint16_t>(data[off]) | (static_cast<uint16_t>(data[off + 1]) << 8));
    };

    auto invert_axis = [](int16_t val) -> int16_t {
        constexpr int AXIS_MIN = -32767;
        constexpr int AXIS_MAX = 32767;
        return static_cast<int16_t>(std::clamp(-static_cast<int>(val), AXIS_MIN, AXIS_MAX));
    };
    state.left_x = read_s16(OFF_LX);
    state.left_y = invert_axis(read_s16(OFF_LY));
    state.right_x = read_s16(OFF_RX);
    state.right_y = invert_axis(read_s16(OFF_RY));
    state.ext_buttons = 0;

    return state;
}

} // namespace vader5
