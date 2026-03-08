#pragma once

#include "config.hpp"
#include "types.hpp"

#include <linux/input.h>

#include <array>
#include <optional>
#include <span>
#include <vector>

namespace vader5 {

struct RumbleEffect {
    uint16_t strong{0};
    uint16_t weak{0};
};

class Uinput {
  public:
    static auto create(std::span<const std::optional<int>> ext_mappings,
                       bool emulate_elite = true,
                       const char* name = "Vader 5 Pro Virtual Gamepad") -> Result<Uinput>;
    ~Uinput();

    Uinput(Uinput&& other) noexcept;
    auto operator=(Uinput&& other) noexcept -> Uinput&;
    Uinput(const Uinput&) = delete;
    auto operator=(const Uinput&) -> Uinput& = delete;

    [[nodiscard]] auto fd() const noexcept -> int {
        return fd_;
    }
    auto emit(const GamepadState& state, const GamepadState& prev) -> Result<void>;
    auto sync() -> Result<void>;
    auto poll_ff() -> std::optional<RumbleEffect>;

  private:
    explicit Uinput(int file_descriptor, std::span<const std::optional<int>> mappings);
    int fd_{-1};
    std::array<std::optional<int>, 8> ext_mappings_{};
    std::array<RumbleEffect, 16> ff_effects_{};
    std::vector<input_event> events_buffer_{};

    void emit_key(int code, int value);
    void emit_abs(int code, int value);
    inline void buffer_event(const input_event& ev);
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

    void move_mouse(int dx, int dy);
    void scroll(int vertical, int horizontal = 0);
    void click(int code, bool pressed);
    void key(int code, bool pressed);
    auto sync() -> Result<void>;

  private:
    explicit InputDevice(int fd) : fd_(fd) {}
    int fd_{-1};
    std::vector<input_event> events_buffer_{};

    void emit_rel(int code, int value);
    void emit_key(int code, int value);
    inline void buffer_event(const input_event& ev);
};

} // namespace vader5
