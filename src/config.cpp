#include "vader5/config.hpp"

#include <linux/input-event-codes.h>
#include <toml++/toml.hpp>

#include <cstdlib>

namespace vader5 {

namespace {
constexpr std::array<std::string_view, 8> EXT_BUTTON_NAMES = {
    "C", "Z", "M1", "M3", "M2", "M4", "LM", "RM"};

struct KeyEntry {
    std::string_view name;
    int code;
};

constexpr std::array KEY_TABLE = {
    // Keyboard keys
    KeyEntry{"KEY_ESC", KEY_ESC},
    KeyEntry{"KEY_1", KEY_1},
    KeyEntry{"KEY_2", KEY_2},
    KeyEntry{"KEY_3", KEY_3},
    KeyEntry{"KEY_4", KEY_4},
    KeyEntry{"KEY_5", KEY_5},
    KeyEntry{"KEY_6", KEY_6},
    KeyEntry{"KEY_7", KEY_7},
    KeyEntry{"KEY_8", KEY_8},
    KeyEntry{"KEY_9", KEY_9},
    KeyEntry{"KEY_0", KEY_0},
    KeyEntry{"KEY_BACKSPACE", KEY_BACKSPACE},
    KeyEntry{"KEY_TAB", KEY_TAB},
    KeyEntry{"KEY_Q", KEY_Q},
    KeyEntry{"KEY_W", KEY_W},
    KeyEntry{"KEY_E", KEY_E},
    KeyEntry{"KEY_R", KEY_R},
    KeyEntry{"KEY_T", KEY_T},
    KeyEntry{"KEY_Y", KEY_Y},
    KeyEntry{"KEY_U", KEY_U},
    KeyEntry{"KEY_I", KEY_I},
    KeyEntry{"KEY_O", KEY_O},
    KeyEntry{"KEY_P", KEY_P},
    KeyEntry{"KEY_ENTER", KEY_ENTER},
    KeyEntry{"KEY_LEFTCTRL", KEY_LEFTCTRL},
    KeyEntry{"KEY_A", KEY_A},
    KeyEntry{"KEY_S", KEY_S},
    KeyEntry{"KEY_D", KEY_D},
    KeyEntry{"KEY_F", KEY_F},
    KeyEntry{"KEY_G", KEY_G},
    KeyEntry{"KEY_H", KEY_H},
    KeyEntry{"KEY_J", KEY_J},
    KeyEntry{"KEY_K", KEY_K},
    KeyEntry{"KEY_L", KEY_L},
    KeyEntry{"KEY_LEFTSHIFT", KEY_LEFTSHIFT},
    KeyEntry{"KEY_Z", KEY_Z},
    KeyEntry{"KEY_X", KEY_X},
    KeyEntry{"KEY_C", KEY_C},
    KeyEntry{"KEY_V", KEY_V},
    KeyEntry{"KEY_B", KEY_B},
    KeyEntry{"KEY_N", KEY_N},
    KeyEntry{"KEY_M", KEY_M},
    KeyEntry{"KEY_RIGHTSHIFT", KEY_RIGHTSHIFT},
    KeyEntry{"KEY_LEFTALT", KEY_LEFTALT},
    KeyEntry{"KEY_SPACE", KEY_SPACE},
    KeyEntry{"KEY_CAPSLOCK", KEY_CAPSLOCK},
    KeyEntry{"KEY_F1", KEY_F1},
    KeyEntry{"KEY_F2", KEY_F2},
    KeyEntry{"KEY_F3", KEY_F3},
    KeyEntry{"KEY_F4", KEY_F4},
    KeyEntry{"KEY_F5", KEY_F5},
    KeyEntry{"KEY_F6", KEY_F6},
    KeyEntry{"KEY_F7", KEY_F7},
    KeyEntry{"KEY_F8", KEY_F8},
    KeyEntry{"KEY_F9", KEY_F9},
    KeyEntry{"KEY_F10", KEY_F10},
    KeyEntry{"KEY_F11", KEY_F11},
    KeyEntry{"KEY_F12", KEY_F12},
    KeyEntry{"KEY_F13", KEY_F13},
    KeyEntry{"KEY_F14", KEY_F14},
    KeyEntry{"KEY_F15", KEY_F15},
    KeyEntry{"KEY_F16", KEY_F16},
    KeyEntry{"KEY_RIGHTCTRL", KEY_RIGHTCTRL},
    KeyEntry{"KEY_RIGHTALT", KEY_RIGHTALT},
    KeyEntry{"KEY_HOME", KEY_HOME},
    KeyEntry{"KEY_UP", KEY_UP},
    KeyEntry{"KEY_PAGEUP", KEY_PAGEUP},
    KeyEntry{"KEY_LEFT", KEY_LEFT},
    KeyEntry{"KEY_RIGHT", KEY_RIGHT},
    KeyEntry{"KEY_END", KEY_END},
    KeyEntry{"KEY_DOWN", KEY_DOWN},
    KeyEntry{"KEY_PAGEDOWN", KEY_PAGEDOWN},
    KeyEntry{"KEY_INSERT", KEY_INSERT},
    KeyEntry{"KEY_DELETE", KEY_DELETE},
    // Gamepad buttons
    KeyEntry{"BTN_SOUTH", BTN_SOUTH},
    KeyEntry{"BTN_EAST", BTN_EAST},
    KeyEntry{"BTN_NORTH", BTN_NORTH},
    KeyEntry{"BTN_WEST", BTN_WEST},
    KeyEntry{"BTN_TL", BTN_TL},
    KeyEntry{"BTN_TR", BTN_TR},
    KeyEntry{"BTN_SELECT", BTN_SELECT},
    KeyEntry{"BTN_START", BTN_START},
    KeyEntry{"BTN_MODE", BTN_MODE},
    KeyEntry{"BTN_THUMBL", BTN_THUMBL},
    KeyEntry{"BTN_THUMBR", BTN_THUMBR},
    // Mouse buttons
    KeyEntry{"BTN_LEFT", BTN_LEFT},
    KeyEntry{"BTN_RIGHT", BTN_RIGHT},
    KeyEntry{"BTN_MIDDLE", BTN_MIDDLE},
    KeyEntry{"mouse_left", BTN_LEFT},
    KeyEntry{"mouse_right", BTN_RIGHT},
    KeyEntry{"mouse_middle", BTN_MIDDLE},
};

auto parse_gyro_mode(std::string_view mode) -> GyroConfig::Mode {
    if (mode == "mouse") return GyroConfig::Mouse;
    if (mode == "joystick") return GyroConfig::Joystick;
    return GyroConfig::Off;
}
} // namespace

auto keycode_from_name(std::string_view name) -> std::optional<int> {
    for (const auto& entry : KEY_TABLE) {
        if (entry.name == name) {
            return entry.code;
        }
    }
    return std::nullopt;
}

auto parse_remap_target(std::string_view value) -> std::optional<RemapTarget> {
    if (value == "disabled") {
        return RemapTarget{RemapTarget::Disabled, 0};
    }
    if (value == "mouse_left") {
        return RemapTarget{RemapTarget::MouseButton, BTN_LEFT};
    }
    if (value == "mouse_right") {
        return RemapTarget{RemapTarget::MouseButton, BTN_RIGHT};
    }
    if (value == "mouse_middle") {
        return RemapTarget{RemapTarget::MouseButton, BTN_MIDDLE};
    }
    if (auto code = keycode_from_name(value)) {
        return RemapTarget{RemapTarget::Key, *code};
    }
    return std::nullopt;
}

auto Config::default_path() -> std::string {
    return "config.toml";
}

namespace {
auto parse_stick_config(const toml::table& tbl, StickConfig& cfg) {
    if (auto* dz = tbl["deadzone"].as_integer()) cfg.deadzone = static_cast<int>(dz->get());
    if (auto* am = tbl["as_mouse"].as_boolean()) cfg.as_mouse = am->get();
    if (auto* ms = tbl["mouse_sensitivity"].as_floating_point()) cfg.mouse_sensitivity = static_cast<float>(ms->get());
}

auto parse_mode_shift(const toml::table& tbl) -> ModeShiftConfig {
    ModeShiftConfig ms;
    if (auto* gyro = tbl["gyro"].as_string()) ms.gyro = parse_gyro_mode(gyro->get());
    if (auto* rsm = tbl["right_stick"].as_string()) ms.right_stick_mouse = (rsm->get() == "mouse");
    if (auto* lsm = tbl["left_stick"].as_string()) ms.left_stick_scroll = (lsm->get() == "scroll");
    if (auto* ss = tbl["scroll_sensitivity"].as_floating_point()) ms.scroll_sensitivity = static_cast<float>(ss->get());
    if (auto* dpad = tbl["dpad"].as_string()) ms.dpad_arrows = (dpad->get() == "arrows");
    for (const auto& [key, val] : tbl) {
        if (key == "gyro" || key == "right_stick" || key == "left_stick" || key == "dpad" || key == "scroll_sensitivity") continue;
        if (auto* str = val.as_string()) {
            if (auto target = parse_remap_target(str->get())) {
                ms.remaps[std::string(key)] = *target;
            }
        }
    }
    return ms;
}
} // namespace

auto Config::load(const std::string& path) -> Result<Config> {
    Config cfg;

    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& /*err*/) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    // [remap] section - button remapping
    if (auto* remap = tbl["remap"].as_table()) {
        for (const auto& [key, val] : *remap) {
            if (auto* str = val.as_string()) {
                // Check if it's an ext button (for backwards compat)
                for (size_t idx = 0; idx < EXT_BUTTON_NAMES.size(); ++idx) {
                    if (key == EXT_BUTTON_NAMES[idx]) {
                        cfg.ext_mappings[idx] = keycode_from_name(str->get());
                        break;
                    }
                }
                // Store in general remaps
                if (auto target = parse_remap_target(str->get())) {
                    cfg.button_remaps[std::string(key)] = *target;
                }
            }
        }
    }

    // [gyro] section
    if (auto* gyro = tbl["gyro"].as_table()) {
        if (auto* mode = (*gyro)["mode"].as_string()) cfg.gyro.mode = parse_gyro_mode(mode->get());
        if (auto* sens = (*gyro)["sensitivity"].as_floating_point()) {
            auto val = static_cast<float>(sens->get());
            cfg.gyro.sensitivity_x = val;
            cfg.gyro.sensitivity_y = val;
        }
        if (auto* sx = (*gyro)["sensitivity_x"].as_floating_point()) cfg.gyro.sensitivity_x = static_cast<float>(sx->get());
        if (auto* sy = (*gyro)["sensitivity_y"].as_floating_point()) cfg.gyro.sensitivity_y = static_cast<float>(sy->get());
        if (auto* dz = (*gyro)["deadzone"].as_integer()) cfg.gyro.deadzone = static_cast<int>(dz->get());
        if (auto* sm = (*gyro)["smoothing"].as_floating_point()) cfg.gyro.smoothing = static_cast<float>(sm->get());
        if (auto* cv = (*gyro)["curve"].as_floating_point()) cfg.gyro.curve = static_cast<float>(cv->get());
        if (auto* ix = (*gyro)["invert_x"].as_boolean()) cfg.gyro.invert_x = ix->get();
        if (auto* iy = (*gyro)["invert_y"].as_boolean()) cfg.gyro.invert_y = iy->get();
    }

    // [stick.left] and [stick.right]
    if (auto* left = tbl["stick"]["left"].as_table()) parse_stick_config(*left, cfg.left_stick);
    if (auto* right = tbl["stick"]["right"].as_table()) parse_stick_config(*right, cfg.right_stick);

    // [mode_shift.XXX] sections
    if (auto* mode_shift = tbl["mode_shift"].as_table()) {
        for (const auto& [key, val] : *mode_shift) {
            if (auto* sub = val.as_table()) {
                cfg.mode_shifts[std::string(key)] = parse_mode_shift(*sub);
            }
        }
    }

    return cfg;
}

} // namespace vader5
