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
    void sync() const;

private:
    explicit Uinput(int file_descriptor, std::span<const std::optional<int>> mappings);
    int fd_{-1};
    std::array<std::optional<int>, 8> ext_mappings_{};

    void emit_key(int code, int value) const;
    void emit_abs(int code, int value) const;
};

// Separate device for mouse/keyboard to avoid Steam detection issues
class InputDevice {
public:
    static auto create(const char* name = "Vader 5 Pro Mouse") -> Result<InputDevice>;
    ~InputDevice();

    InputDevice(InputDevice&& other) noexcept;
    auto operator=(InputDevice&& other) noexcept -> InputDevice&;
    InputDevice(const InputDevice&) = delete;
    auto operator=(const InputDevice&) -> InputDevice& = delete;

    void move_mouse(int dx, int dy) const;
    void scroll(int vertical, int horizontal = 0) const;
    void click(int code, bool pressed) const;
    void key(int code, bool pressed) const;
    void sync() const;

private:
    explicit InputDevice(int fd) : fd_(fd) {}
    int fd_{-1};

    void emit_rel(int code, int value) const;
    void emit_key(int code, int value) const;
};

} // namespace vader5
