#pragma once

#include "config.hpp"
#include "hidraw.hpp"
#include "uinput.hpp"

#include <unistd.h>

#include <chrono>
#include <unordered_set>
#include <utility>

namespace vader5 {

struct TapHoldState {
    std::string layer_name;
    std::chrono::steady_clock::time_point press_time;
    bool layer_activated{false};
};

class UniqueFd {
    int fd_;

  public:
    explicit constexpr UniqueFd(int fd) noexcept : fd_{fd} {}

    ~UniqueFd() {
        if (fd_ >= 0) {
            ::close(fd_);
        }
    }

    UniqueFd() = delete;
    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;
    UniqueFd& operator=(UniqueFd&&) = delete;

    constexpr UniqueFd(UniqueFd&& other) noexcept : fd_{std::exchange(other.fd_, -1)} {}

    [[nodiscard]] constexpr int get() const noexcept {
        return fd_;
    }
};

class Gamepad {
  public:
    static auto open(const Config& cfg, const std::string& device_name) -> Result<Gamepad>;
    ~Gamepad();

    Gamepad(Gamepad&&) = default;
    Gamepad& operator=(Gamepad&&) = delete;
    Gamepad(const Gamepad&) = delete;
    Gamepad& operator=(const Gamepad&) = delete;

    auto poll() -> Result<void>;
    void poll_ff();
    auto send_rumble(uint8_t left, uint8_t right) -> bool;
    [[nodiscard]] auto fd() const noexcept -> int {
        return hidraw_.fd();
    }
    [[nodiscard]] auto ff_fd() const noexcept -> int {
        return uinput_.fd();
    }

  private:
    Gamepad(Hidraw&& hid, Uinput&& uinput, std::optional<InputDevice>&& input, UniqueFd&& redundant,
            Config cfg)
        : hidraw_(std::move(hid)), uinput_(std::move(uinput)), input_(std::move(input)),
          redundant_(std::move(redundant)), config_(std::move(cfg)) {}

    void process_gyro(const GamepadState& state);
    void process_mouse_stick(const GamepadState& state);
    void process_scroll_stick(const GamepadState& state);
    void process_layer_dpad(const GamepadState& state);
    void process_layer_buttons(const GamepadState& state, const GamepadState& prev);
    void process_base_remaps(const GamepadState& state, const GamepadState& prev);
    void update_tap_hold(const GamepadState& state, const GamepadState& prev);
    void emit_tap(const RemapTarget& tap);
    auto get_active_layer() -> const LayerConfig*;
    static auto is_button_pressed(const GamepadState& state, std::string_view name) -> bool;

    auto get_effective_gyro() -> const GyroConfig&;
    auto get_effective_stick_left() -> const StickConfig&;
    auto get_effective_stick_right() -> const StickConfig&;
    auto get_effective_dpad() -> const DpadConfig&;

    Hidraw hidraw_;
    Uinput uinput_;
    std::optional<InputDevice> input_;
    UniqueFd redundant_;
    Config config_;
    GamepadState prev_state_{};
    std::unordered_map<std::string, TapHoldState> tap_hold_states_;
    std::unordered_set<std::string> toggled_layers_;
    float gyro_vel_x_{0.0F};
    float gyro_vel_y_{0.0F};
    float gyro_accum_x_{0.0F};
    float gyro_accum_y_{0.0F};
    float scroll_accum_v_{0.0F};
    float scroll_accum_h_{0.0F};
    int gyro_stick_x_{0};
    int gyro_stick_y_{0};
    bool dpad_up_{false};
    bool dpad_down_{false};
    bool dpad_left_{false};
    bool dpad_right_{false};
    uint16_t suppressed_buttons_{0};
    uint8_t suppressed_ext_{0};
    uint16_t prev_suppressed_buttons_{0};
    uint8_t prev_suppressed_ext_{0};
    uint16_t injected_buttons_{0};
    uint8_t injected_ext_{0};
    uint16_t prev_injected_buttons_{0};
    uint8_t prev_injected_ext_{0};
};

} // namespace vader5
