#pragma once

#include "types.hpp"

#include <optional>
#include <span>
#include <string>

// Forward declaration
struct hid_device_;

namespace vader5 {

class Hidraw {
  public:
    static auto open(uint16_t vid, uint16_t pid, int iface = 0, const std::string& device_name = "") -> Result<Hidraw>;
    ~Hidraw();
    auto reconnect(uint16_t vid, uint16_t pid, int iface = 0) -> Result<void>;

    Hidraw(Hidraw&& other) noexcept;
    auto operator=(Hidraw&& other) noexcept -> Hidraw&;
    Hidraw(const Hidraw&) = delete;
    auto operator=(const Hidraw&) -> Hidraw& = delete;

    // Returns a dummy fd for compatibility. The real I/O goes through hidapi.
    // This fd should NOT be used for read/write/poll - only for existence checks.
    [[nodiscard]] auto fd() const noexcept -> int {
        return device_ ? 0 : -1;
    }
    [[nodiscard]] auto phys() const -> Result<std::string>;
    [[nodiscard]] auto read(std::span<uint8_t> buf) const -> Result<size_t>;
    [[nodiscard]] auto write(std::span<const uint8_t> buf) const -> Result<size_t>;
    [[nodiscard]] static auto parse_report(std::span<const uint8_t> data)
        -> std::optional<GamepadState>;

  private:
    explicit Hidraw(struct hid_device_* device) : device_(device) {}
    struct hid_device_* device_{nullptr};

    [[nodiscard]] static auto parse_report_24g(std::span<const uint8_t> data)
        -> std::optional<GamepadState>;
};

auto find_hidraw_device(uint16_t vid, uint16_t pid, int iface) -> Result<std::string>;

} // namespace vader5
