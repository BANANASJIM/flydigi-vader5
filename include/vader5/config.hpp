#pragma once

#include "types.hpp"

#include <array>
#include <optional>
#include <string>
#include <unordered_map>

namespace vader5 {

// Target for button remapping
struct RemapTarget {
    enum Type { Disabled, Key, MouseButton, MouseMove };
    Type type{Key};
    int code{0};
};

struct GyroConfig {
    enum Mode { Off, Mouse, Joystick };
    Mode mode{Off};
    float sensitivity_x{1.5f};
    float sensitivity_y{1.5f};
    int deadzone{0};
    float smoothing{0.3f};
    float curve{1.0f}; // 1.0=linear, <1=slow start, >1=fast start
    bool invert_x{false};
    bool invert_y{false};
};

struct StickConfig {
    int deadzone{128};
    bool as_mouse{false};
    float mouse_sensitivity{1.0f};
};

struct ModeShiftConfig {
    GyroConfig::Mode gyro{GyroConfig::Off};
    bool right_stick_mouse{false};
    bool left_stick_scroll{false};
    float scroll_sensitivity{1.0f};
    bool dpad_arrows{false};
    std::unordered_map<std::string, RemapTarget> remaps;
};

struct Config {
    std::array<std::optional<int>, 8> ext_mappings{};
    std::unordered_map<std::string, RemapTarget> button_remaps;
    GyroConfig gyro;
    StickConfig left_stick;
    StickConfig right_stick;
    std::unordered_map<std::string, ModeShiftConfig> mode_shifts;

    static auto load(const std::string& path) -> Result<Config>;
    static auto default_path() -> std::string;
};

auto keycode_from_name(std::string_view name) -> std::optional<int>;
auto parse_remap_target(std::string_view value) -> std::optional<RemapTarget>;

} // namespace vader5
