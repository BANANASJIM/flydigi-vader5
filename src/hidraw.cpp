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
                    const auto vendor_str =
                        line.substr(pos + HID_ID_PREFIX_LEN, HID_ID_FIELD_LEN);
                    const auto product_str =
                        line.substr(pos + HID_ID_PRODUCT_OFFSET, HID_ID_FIELD_LEN);
                    const auto parsed_vid = std::stoul(vendor_str, nullptr, 16);
                    const auto parsed_pid = std::stoul(product_str, nullptr, 16);
                    vid_match = (parsed_vid == vid);
                    pid_match = (parsed_pid == pid);
                }
            }
            if (iface_num >= 0 && line.starts_with("HID_PHYS=")) {
                constexpr std::string_view INPUT_PREFIX = "/input";
                const auto pos = line.rfind(INPUT_PREFIX);
                if (pos != std::string::npos) {
                    const auto iface_str = line.substr(pos + INPUT_PREFIX.size());
                    iface_match = (std::stoi(iface_str) == iface_num);
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

    const int file_descriptor = ::open(dev_path->c_str(), O_RDONLY | O_NONBLOCK);
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

auto Hidraw::parse_report(std::span<const uint8_t> data) -> std::optional<GamepadState> {
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

} // namespace vader5
