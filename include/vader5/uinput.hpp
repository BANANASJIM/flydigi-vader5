#pragma once

#include "config.hpp"
#include "types.hpp"

#include <array>
#include <optional>
#include <span>

namespace vader5 {

class Uinput {
public:
    static auto create(std::span<const std::optional<int>> ext_mappings,
                       const char* name = "Vader 5 Pro Virtual Gamepad") -> Result<Uinput>;
    ~Uinput();

    Uinput(Uinput&& other) noexcept;
    auto operator=(Uinput&& other) noexcept -> Uinput&;
    Uinput(const Uinput&) = delete;
    auto operator=(const Uinput&) -> Uinput& = delete;

    [[nodiscard]] auto fd() const noexcept -> int { return fd_; }
    auto emit(const GamepadState& state, const GamepadState& prev) const -> Result<void>;

private:
    explicit Uinput(int file_descriptor, std::span<const std::optional<int>> mappings);
    int fd_{-1};
    std::array<std::optional<int>, 8> ext_mappings_{};

    void emit_key(int code, int value) const;
    void emit_abs(int code, int value) const;
    void sync() const;
};

} // namespace vader5
