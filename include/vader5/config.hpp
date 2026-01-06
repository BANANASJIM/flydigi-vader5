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
    float sensitivity_x{1.5F};
    float sensitivity_y{1.5F};
    int deadzone{0};
    float smoothing{0.3F};
    float curve{1.0F};
    bool invert_x{false};
    bool invert_y{false};
};

struct StickConfig {
    enum Mode { Gamepad, Mouse, Scroll };
    Mode mode{Gamepad};
    int deadzone{128};
    float sensitivity{1.0F};
};

struct DpadConfig {
    enum Mode { Gamepad, Arrows };
    Mode mode{Gamepad};
};

struct LedConfig {
    enum Mode { Off, Static, Breathing };
    Mode mode{Static};
    uint8_t brightness{80};
    uint8_t r{0}, g{255}, b{0};
};

struct LayerConfig {
    enum Activation { Hold, Toggle };
    std::string name;
    std::string trigger;
    std::optional<RemapTarget> tap;
    int hold_timeout{200};
    Activation activation{Hold};

    std::optional<GyroConfig> gyro;
    std::optional<StickConfig> stick_left;
    std::optional<StickConfig> stick_right;
    std::optional<DpadConfig> dpad;
    std::optional<LedConfig> led;
    std::unordered_map<std::string, RemapTarget> remap;
};

struct Config {
    bool emulate_elite{true};
    std::array<std::optional<int>, 8> ext_mappings{};
    std::unordered_map<std::string, RemapTarget> button_remaps;
    GyroConfig gyro;
    StickConfig left_stick;
    StickConfig right_stick;
    DpadConfig dpad;
    LedConfig led;
    std::unordered_map<std::string, LayerConfig> layers;

    static auto load(const std::string& path) -> Result<Config>;
    static auto default_path() -> std::string;
};

auto parse_remap_target(std::string_view value) -> std::optional<RemapTarget>;

} // namespace vader5
