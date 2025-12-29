#pragma once

#include "types.hpp"

#include <optional>
#include <span>
#include <string>

namespace vader5 {

class Hidraw {
public:
    static auto open(uint16_t vid, uint16_t pid, int iface = 0) -> Result<Hidraw>;
    ~Hidraw();

    Hidraw(Hidraw&& other) noexcept;
    auto operator=(Hidraw&& other) noexcept -> Hidraw&;
    Hidraw(const Hidraw&) = delete;
    auto operator=(const Hidraw&) -> Hidraw& = delete;

    [[nodiscard]] auto fd() const noexcept -> int { return fd_; }
    [[nodiscard]] auto read(std::span<uint8_t> buf) const -> Result<size_t>;
    [[nodiscard]] static auto parse_report(std::span<const uint8_t> data)
        -> std::optional<GamepadState>;

private:
    explicit Hidraw(int file_descriptor) : fd_(file_descriptor) {}
    int fd_{-1};
};

auto find_hidraw_device(uint16_t vid, uint16_t pid, int iface) -> Result<std::string>;

} // namespace vader5
