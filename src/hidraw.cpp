#include "vader5/hidraw.hpp"
#include "vader5/protocol.hpp"

#include <fcntl.h>
#include <linux/hidraw.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>

namespace vader5 {
namespace fs = std::filesystem;

constexpr uint8_t DPAD_MASK = 0x0F;

auto find_hidraw_device(uint16_t vid, uint16_t pid, int iface_num) -> Result<std::string> {
    for (const auto& entry : fs::directory_iterator("/sys/class/hidraw")) {
        const auto uevent_path = entry.path() / "device" / "uevent";
        std::ifstream uevent(uevent_path);
        if (!uevent) {
            continue;
        }

        std::string line;
        bool vid_match = false;
        bool pid_match = false;
        bool iface_match = (iface_num < 0);

        while (std::getline(uevent, line)) {
            try {
                if (line.starts_with("HID_ID=")) {
                    if (auto pos = line.find(':'); pos != std::string::npos) {
                        vid_match = (std::stoul(line.substr(pos + 1, 8), nullptr, 16) == vid);
                        pid_match = (std::stoul(line.substr(pos + 10, 8), nullptr, 16) == pid);
                    }
                }
                if (iface_num >= 0 && line.starts_with("HID_PHYS=")) {
                    if (auto pos = line.rfind("/input"); pos != std::string::npos) {
                        iface_match = (std::stoi(line.substr(pos + 6)) == iface_num);
                    }
                }
            } catch (const std::exception&) {
                continue;
            }
        }

        if (vid_match && pid_match && iface_match) {
            return "/dev/" + entry.path().filename().string();
        }
    }
    return std::unexpected(std::make_error_code(std::errc::no_such_device));
}

auto Hidraw::open(uint16_t vid, uint16_t pid, int iface) -> Result<Hidraw> {
    auto path = find_hidraw_device(vid, pid, iface);
    if (!path) {
        return std::unexpected(path.error());
    }

    const int fd = ::open(path->c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    return Hidraw(fd);
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
    const auto bytes = ::read(fd_, buf.data(), buf.size());
    if (bytes < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }
    return static_cast<size_t>(bytes);
}

auto Hidraw::write(std::span<const uint8_t> buf) const -> Result<size_t> {
    const auto bytes = ::write(fd_, buf.data(), buf.size());
    if (bytes < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }
    return static_cast<size_t>(bytes);
}

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