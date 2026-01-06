#include "vader5/config.hpp"
#include "vader5/debug.hpp"
#include "vader5/keycodes.hpp"

#include <linux/input-event-codes.h>
#include <toml++/toml.hpp>

#include <iostream>

namespace vader5 {

namespace {
constexpr std::array<std::string_view, 8> EXT_BUTTON_NAMES = {"C",  "Z",  "M1", "M2",
                                                              "M3", "M4", "LM", "RM"};

auto parse_gyro_mode(std::string_view mode) -> GyroConfig::Mode {
    if (mode == "mouse") {
        return GyroConfig::Mouse;
    }
    if (mode == "joystick") {
        return GyroConfig::Joystick;
    }
    return GyroConfig::Off;
}

auto parse_stick_mode(std::string_view mode) -> StickConfig::Mode {
    if (mode == "mouse") {
        return StickConfig::Mouse;
    }
    if (mode == "scroll") {
        return StickConfig::Scroll;
    }
    return StickConfig::Gamepad;
}

auto parse_dpad_mode(std::string_view mode) -> DpadConfig::Mode {
    return mode == "arrows" ? DpadConfig::Arrows : DpadConfig::Gamepad;
}

auto parse_led_mode(std::string_view mode) -> LedConfig::Mode {
    if (mode == "off") {
        return LedConfig::Off;
    }
    if (mode == "breathing") {
        return LedConfig::Breathing;
    }
    return LedConfig::Static;
}

auto parse_hex_color(std::string_view color, uint8_t& r, uint8_t& g, uint8_t& b) -> bool {
    if (color.size() != 7 || color[0] != '#') {
        return false;
    }
    auto hex = [](char c) -> int {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        return -1;
    };
    const int rh = hex(color[1]);
    const int rl = hex(color[2]);
    const int gh = hex(color[3]);
    const int gl = hex(color[4]);
    const int bh = hex(color[5]);
    const int bl = hex(color[6]);
    if (rh < 0 || rl < 0 || gh < 0 || gl < 0 || bh < 0 || bl < 0) {
        return false;
    }
    r = static_cast<uint8_t>((rh << 4) | rl);
    g = static_cast<uint8_t>((gh << 4) | gl);
    b = static_cast<uint8_t>((bh << 4) | bl);
    return true;
}

void parse_led(const toml::table& tbl, LedConfig& cfg) {
    if (const auto* val = tbl["mode"].as_string()) {
        cfg.mode = parse_led_mode(val->get());
    }
    if (const auto* val = tbl["brightness"].as_integer()) {
        cfg.brightness = static_cast<uint8_t>(std::clamp<int64_t>(val->get(), 0, 100));
    }
    if (const auto* val = tbl["color"].as_string()) {
        parse_hex_color(val->get(), cfg.r, cfg.g, cfg.b);
    }
}

void parse_gyro(const toml::table& tbl, GyroConfig& cfg) {
    if (const auto* val = tbl["mode"].as_string()) {
        cfg.mode = parse_gyro_mode(val->get());
    }
    if (const auto* val = tbl["sensitivity"].as_floating_point()) {
        cfg.sensitivity_x = cfg.sensitivity_y = static_cast<float>(val->get());
    }
    if (const auto* val = tbl["sensitivity_x"].as_floating_point()) {
        cfg.sensitivity_x = static_cast<float>(val->get());
    }
    if (const auto* val = tbl["sensitivity_y"].as_floating_point()) {
        cfg.sensitivity_y = static_cast<float>(val->get());
    }
    if (const auto* val = tbl["deadzone"].as_integer()) {
        cfg.deadzone = static_cast<int>(val->get());
    }
    if (const auto* val = tbl["smoothing"].as_floating_point()) {
        cfg.smoothing = static_cast<float>(val->get());
    }
    if (const auto* val = tbl["curve"].as_floating_point()) {
        cfg.curve = static_cast<float>(val->get());
    }
    if (const auto* val = tbl["invert_x"].as_boolean()) {
        cfg.invert_x = val->get();
    }
    if (const auto* val = tbl["invert_y"].as_boolean()) {
        cfg.invert_y = val->get();
    }
}

void parse_stick(const toml::table& tbl, StickConfig& cfg) {
    if (const auto* val = tbl["mode"].as_string()) {
        cfg.mode = parse_stick_mode(val->get());
    }
    if (const auto* val = tbl["deadzone"].as_integer()) {
        cfg.deadzone = static_cast<int>(val->get());
    }
    if (const auto* val = tbl["sensitivity"].as_floating_point()) {
        cfg.sensitivity = static_cast<float>(val->get());
    }
}

void parse_dpad(const toml::table& tbl, DpadConfig& cfg) {
    if (const auto* val = tbl["mode"].as_string()) {
        cfg.mode = parse_dpad_mode(val->get());
    }
}

auto parse_layer(const std::string& name, const toml::table& tbl) -> LayerConfig {
    LayerConfig layer;
    layer.name = name;

    if (const auto* val = tbl["trigger"].as_string()) {
        layer.trigger = val->get();
    }
    if (const auto* val = tbl["tap"].as_string()) {
        layer.tap = parse_remap_target(val->get());
    }
    if (const auto* val = tbl["hold_timeout"].as_integer()) {
        layer.hold_timeout = static_cast<int>(val->get());
    }
    if (const auto* val = tbl["activation"].as_string()) {
        layer.activation = (val->get() == "toggle") ? LayerConfig::Toggle : LayerConfig::Hold;
    }
    if (const auto* sub = tbl["gyro"].as_table()) {
        GyroConfig gc;
        parse_gyro(*sub, gc);
        layer.gyro = gc;
    }
    if (const auto* sub = tbl["stick_left"].as_table()) {
        StickConfig sc;
        parse_stick(*sub, sc);
        layer.stick_left = sc;
    }
    if (const auto* sub = tbl["stick_right"].as_table()) {
        StickConfig sc;
        parse_stick(*sub, sc);
        layer.stick_right = sc;
    }
    if (const auto* sub = tbl["dpad"].as_table()) {
        DpadConfig dc;
        parse_dpad(*sub, dc);
        layer.dpad = dc;
    }
    if (const auto* sub = tbl["led"].as_table()) {
        LedConfig lc;
        parse_led(*sub, lc);
        layer.led = lc;
    }
    if (const auto* sub = tbl["remap"].as_table()) {
        for (const auto& [key, node] : *sub) {
            if (const auto* str = node.as_string()) {
                if (auto target = parse_remap_target(str->get())) {
                    layer.remap[std::string(key)] = *target;
                }
            }
        }
    }
    return layer;
}

void detect_conflicts(const Config& cfg) {
    for (const auto& [name, layer] : cfg.layers) {
        if (cfg.button_remaps.contains(layer.trigger)) {
            std::cerr << "[WARN] " << layer.trigger << " is trigger for '" << name
                      << "', base [remap] will be ignored for this button\n";
        }
    }
}
} // namespace

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
    if (value == "mouse_side") {
        return RemapTarget{RemapTarget::MouseButton, BTN_SIDE};
    }
    if (value == "mouse_extra") {
        return RemapTarget{RemapTarget::MouseButton, BTN_EXTRA};
    }
    if (value == "mouse_forward") {
        return RemapTarget{RemapTarget::MouseButton, BTN_FORWARD};
    }
    if (value == "mouse_back") {
        return RemapTarget{RemapTarget::MouseButton, BTN_BACK};
    }
    if (auto code = keycode_from_name(value)) {
        return RemapTarget{RemapTarget::Key, *code};
    }
    return std::nullopt;
}

auto Config::default_path() -> std::string {
    return "config/config.toml";
}

auto Config::load(const std::string& path) -> Result<Config> {
    Config cfg;
    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error&) {
        return std::unexpected(std::make_error_code(std::errc::invalid_argument));
    }

    if (const auto* val = tbl["emulate_elite"].as_boolean()) {
        cfg.emulate_elite = val->get();
    }

    if (const auto* remap_tbl = tbl["remap"].as_table()) {
        for (const auto& [key, node] : *remap_tbl) {
            if (const auto* str = node.as_string()) {
                for (size_t idx = 0; idx < EXT_BUTTON_NAMES.size(); ++idx) {
                    if (key == EXT_BUTTON_NAMES[idx]) {
                        cfg.ext_mappings[idx] = keycode_from_name(str->get());
                        break;
                    }
                }
                if (auto target = parse_remap_target(str->get())) {
                    cfg.button_remaps[std::string(key)] = *target;
                }
            }
        }
    }

    if (const auto* gyro_tbl = tbl["gyro"].as_table()) {
        parse_gyro(*gyro_tbl, cfg.gyro);
    }
    if (const auto* left_tbl = tbl["stick"]["left"].as_table()) {
        parse_stick(*left_tbl, cfg.left_stick);
    }
    if (const auto* right_tbl = tbl["stick"]["right"].as_table()) {
        parse_stick(*right_tbl, cfg.right_stick);
    }
    if (const auto* dpad_tbl = tbl["dpad"].as_table()) {
        parse_dpad(*dpad_tbl, cfg.dpad);
    }
    if (const auto* led_tbl = tbl["led"].as_table()) {
        parse_led(*led_tbl, cfg.led);
    }

    if (const auto* layer_tbl = tbl["layer"].as_table()) {
        for (const auto& [name, node] : *layer_tbl) {
            if (const auto* sub = node.as_table()) {
                cfg.layers[std::string(name)] = parse_layer(std::string(name), *sub);
            }
        }
    }

    if (const auto* shift_tbl = tbl["mode_shift"].as_table()) {
        for (const auto& [name, node] : *shift_tbl) {
            if (const auto* sub = node.as_table()) {
                std::cerr << "[WARN] [mode_shift." << name << "] deprecated, use [layer."
                          << name << "]\n";
                auto layer = parse_layer(std::string(name), *sub);
                if (layer.trigger.empty()) {
                    layer.trigger = std::string(name);
                }
                cfg.layers[std::string(name)] = std::move(layer);
            }
        }
    }

    detect_conflicts(cfg);

#ifndef NDEBUG
    DBG("emulate_elite = " << (cfg.emulate_elite ? "true" : "false"));
    DBG("button_remaps count: " << cfg.button_remaps.size());
    for (const auto& [btn, target] : cfg.button_remaps) {
        DBG("  " << btn << " -> type=" << static_cast<int>(target.type) << " code=" << target.code);
    }
#endif

    return cfg;
}

} // namespace vader5
