#pragma once

#include "config.hpp"
#include "hidraw.hpp"
#include "uinput.hpp"

namespace vader5 {

class Gamepad {
  public:
    static auto open(const Config& cfg) -> Result<Gamepad>;
    ~Gamepad();

    Gamepad(Gamepad&&) = default;
    Gamepad& operator=(Gamepad&&) = default;
    Gamepad(const Gamepad&) = delete;
    Gamepad& operator=(const Gamepad&) = delete;

    auto poll() -> Result<void>;
    auto send_rumble(uint8_t left, uint8_t right) -> bool;
    [[nodiscard]] auto fd() const noexcept -> int {
        return hidraw_.fd();
    }

  private:
    Gamepad(Hidraw&& hid, Uinput&& uinput, std::optional<InputDevice>&& input, Config cfg)
        : hidraw_(std::move(hid)), uinput_(std::move(uinput)), input_(std::move(input)),
          config_(std::move(cfg)) {}

    void process_gyro(const GamepadState& state);
    void process_mouse_stick(const GamepadState& state);
    void process_scroll_stick(const GamepadState& state);
    void process_mode_shift_dpad(const GamepadState& state, const GamepadState& prev);
    void process_mode_shift_buttons(const GamepadState& state, const GamepadState& prev);
    auto get_active_mode_shift(const GamepadState& state) -> const ModeShiftConfig*;
    static auto is_button_pressed(const GamepadState& state, std::string_view name) -> bool;

    Hidraw hidraw_;
    Uinput uinput_;
    std::optional<InputDevice> input_;
    Config config_;
    GamepadState prev_state_{};
    float gyro_vel_x_{0.0f};
    float gyro_vel_y_{0.0f};
    float gyro_accum_x_{0.0f};
    float gyro_accum_y_{0.0f};
    float scroll_accum_v_{0.0f};
    float scroll_accum_h_{0.0f};
    bool dpad_up_{false};
    bool dpad_down_{false};
    bool dpad_left_{false};
    bool dpad_right_{false};
};

} // namespace vader5
