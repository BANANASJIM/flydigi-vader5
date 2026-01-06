#include "vader5/gamepad.hpp"
#include "vader5/debug.hpp"
#include "vader5/protocol.hpp"

#include <linux/input-event-codes.h>

#include <array>
#include <cmath>
#include <thread>

namespace vader5 {

namespace {
constexpr int CONFIG_INTERFACE = 1;
constexpr float GYRO_SCALE = 0.001F;
constexpr float STICK_SCALE = 0.0001F;
constexpr float SCROLL_SCALE = 0.00005F;
constexpr float GYRO_MAX = 32768.0F;
constexpr int AXIS_MAX = 32767;

constexpr auto button_to_masks(std::string_view name) -> std::pair<uint16_t, uint8_t> {
    if (name == "A") { return {PAD_A, 0}; }
    if (name == "B") { return {PAD_B, 0}; }
    if (name == "X") { return {PAD_X, 0}; }
    if (name == "Y") { return {PAD_Y, 0}; }
    if (name == "LB") { return {PAD_LB, 0}; }
    if (name == "RB") { return {PAD_RB, 0}; }
    if (name == "SELECT") { return {PAD_SELECT, 0}; }
    if (name == "START") { return {PAD_START, 0}; }
    if (name == "L3") { return {PAD_L3, 0}; }
    if (name == "R3") { return {PAD_R3, 0}; }
    if (name == "C") { return {0, EXT_C}; }
    if (name == "Z") { return {0, EXT_Z}; }
    if (name == "M1") { return {0, EXT_M1}; }
    if (name == "M2") { return {0, EXT_M2}; }
    if (name == "M3") { return {0, EXT_M3}; }
    if (name == "M4") { return {0, EXT_M4}; }
    if (name == "LM") { return {0, EXT_LM}; }
    if (name == "RM") { return {0, EXT_RM}; }
    return {0, 0};
}

auto apply_curve(float value, float curve, float deadzone = 0.0F) -> float {
    if (curve == 1.0F) {
        return value;
    }
    const float abs_val = std::abs(value);
    if (abs_val <= deadzone) {
        return value;
    }
    const float range = GYRO_MAX - deadzone;
    const float normalized = std::clamp((abs_val - deadzone) / range, 0.0F, 1.0F);
    const float curved = std::pow(normalized, curve);
    const float result = (curved * range) + deadzone;
    return std::copysign(result, value);
}

constexpr uint8_t CMD_TEST_MODE = 0x11;
constexpr uint8_t CMD_RUMBLE = 0x12;
constexpr uint8_t CHECKSUM_TEST_ON = 0x15;
constexpr uint8_t CHECKSUM_TEST_OFF = 0x14;

constexpr std::array<uint8_t, 5> CMD_01 = {0x5a, 0xa5, 0x01, 0x02, 0x03};
constexpr std::array<uint8_t, 5> CMD_A1 = {0x5a, 0xa5, 0xa1, 0x02, 0xa3};
constexpr std::array<uint8_t, 5> CMD_02 = {0x5a, 0xa5, 0x02, 0x02, 0x04};
constexpr std::array<uint8_t, 5> CMD_04 = {0x5a, 0xa5, 0x04, 0x02, 0x06};

auto send_cmd(Hidraw& hid, std::span<const uint8_t> cmd) -> bool {
    std::array<uint8_t, PKT_SIZE> pkt{};
    std::ranges::copy(cmd, pkt.begin());
    if (!hid.write(pkt)) {
        return false;
    }
    std::array<uint8_t, PKT_SIZE> resp{};
    for (int retry = 0; retry < 10; ++retry) {
        const auto result = hid.read(resp);
        if (result && *result >= 4 && resp.at(0) == MAGIC_5A && resp.at(1) == MAGIC_A5) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
}

auto drain_buffer(Hidraw& hid) -> void {
    std::array<uint8_t, PKT_SIZE> buf{};
    for (int count = 0; count < 10 && hid.read(buf).value_or(0) > 0; ++count) {
    }
}

auto send_init(Hidraw& hid) -> bool {
    drain_buffer(hid);
    return send_cmd(hid, CMD_01) && send_cmd(hid, CMD_A1) && send_cmd(hid, CMD_02) &&
           send_cmd(hid, CMD_04);
}

auto send_test_mode(Hidraw& hid, bool enable) -> bool {
    std::array<uint8_t, PKT_SIZE> pkt{};
    pkt.at(0) = MAGIC_5A;
    pkt.at(1) = MAGIC_A5;
    pkt.at(2) = CMD_TEST_MODE;
    pkt.at(3) = 0x07;
    pkt.at(4) = 0xff;
    pkt.at(5) = enable ? 0x01 : 0x00;
    pkt.at(6) = 0xff;
    pkt.at(7) = 0xff;
    pkt.at(8) = 0xff;
    pkt.at(9) = enable ? CHECKSUM_TEST_ON : CHECKSUM_TEST_OFF;
    return hid.write(pkt).has_value();
}

auto needs_mouse(const Config& cfg) -> bool {
    if (cfg.gyro.mode == GyroConfig::Mouse) {
        return true;
    }
    if (cfg.left_stick.mode == StickConfig::Mouse || cfg.left_stick.mode == StickConfig::Scroll) {
        return true;
    }
    if (cfg.right_stick.mode == StickConfig::Mouse) {
        return true;
    }
    if (cfg.dpad.mode == DpadConfig::Arrows) {
        return true;
    }
    if (!cfg.emulate_elite) {
        for (const auto& [btn, target] : cfg.button_remaps) {
            (void)btn;
            if (target.type == RemapTarget::MouseButton || target.type == RemapTarget::Key) {
                return true;
            }
        }
    }
    for (const auto& [name, layer] : cfg.layers) {
        (void)name;
        if (layer.gyro && layer.gyro->mode == GyroConfig::Mouse) {
            return true;
        }
        if (layer.stick_right && layer.stick_right->mode == StickConfig::Mouse) {
            return true;
        }
        if (layer.stick_left && layer.stick_left->mode == StickConfig::Scroll) {
            return true;
        }
        if (layer.dpad && layer.dpad->mode == DpadConfig::Arrows) {
            return true;
        }
        for (const auto& [btn, target] : layer.remap) {
            (void)btn;
            if (target.type == RemapTarget::MouseButton || target.type == RemapTarget::Key) {
                return true;
            }
        }
        if (layer.tap && (layer.tap->type == RemapTarget::Key ||
                          layer.tap->type == RemapTarget::MouseButton)) {
            return true;
        }
    }
    return false;
}
} // namespace

auto Gamepad::open(const Config& cfg) -> Result<Gamepad> {
    auto hid = Hidraw::open(VENDOR_ID, PRODUCT_ID, CONFIG_INTERFACE);
    if (!hid) {
        return std::unexpected(hid.error());
    }

    if (!send_init(*hid) || !send_test_mode(*hid, true)) {
        return std::unexpected(std::make_error_code(std::errc::protocol_error));
    }

    auto uinput = Uinput::create(cfg.ext_mappings);
    if (!uinput) {
        send_test_mode(*hid, false);
        return std::unexpected(uinput.error());
    }

    std::optional<InputDevice> input;
    if (needs_mouse(cfg)) {
        auto dev = InputDevice::create();
        if (!dev) {
            send_test_mode(*hid, false);
            return std::unexpected(dev.error());
        }
        input = std::move(*dev);
    }

    return Gamepad(std::move(*hid), std::move(*uinput), std::move(input), cfg);
}

auto Gamepad::is_button_pressed(const GamepadState& state, std::string_view name) -> bool {
    if (name == "LM") {
        return (state.ext_buttons & EXT_LM) != 0;
    }
    if (name == "RM") {
        return (state.ext_buttons & EXT_RM) != 0;
    }
    if (name == "C") {
        return (state.ext_buttons & EXT_C) != 0;
    }
    if (name == "Z") {
        return (state.ext_buttons & EXT_Z) != 0;
    }
    if (name == "M1") {
        return (state.ext_buttons & EXT_M1) != 0;
    }
    if (name == "M2") {
        return (state.ext_buttons & EXT_M2) != 0;
    }
    if (name == "M3") {
        return (state.ext_buttons & EXT_M3) != 0;
    }
    if (name == "M4") {
        return (state.ext_buttons & EXT_M4) != 0;
    }
    if (name == "A") {
        return (state.buttons & PAD_A) != 0;
    }
    if (name == "B") {
        return (state.buttons & PAD_B) != 0;
    }
    if (name == "X") {
        return (state.buttons & PAD_X) != 0;
    }
    if (name == "Y") {
        return (state.buttons & PAD_Y) != 0;
    }
    if (name == "RB") {
        return (state.buttons & PAD_RB) != 0;
    }
    if (name == "LB") {
        return (state.buttons & PAD_LB) != 0;
    }
    if (name == "START") {
        return (state.buttons & PAD_START) != 0;
    }
    if (name == "SELECT") {
        return (state.buttons & PAD_SELECT) != 0;
    }
    if (name == "L3") {
        return (state.buttons & PAD_L3) != 0;
    }
    if (name == "R3") {
        return (state.buttons & PAD_R3) != 0;
    }
    if (name == "RT") {
        return state.right_trigger > 128;
    }
    if (name == "LT") {
        return state.left_trigger > 128;
    }
    return false;
}

auto Gamepad::get_active_layer() -> const LayerConfig* {
    for (const auto& [name, layer] : config_.layers) {
        if (toggled_layers_.contains(name)) {
            return &layer;
        }
        auto it = tap_hold_states_.find(name);
        if (it != tap_hold_states_.end() && it->second.layer_activated) {
            return &layer;
        }
    }
    return nullptr;
}

void Gamepad::emit_tap(const RemapTarget& tap) {
    if (!input_) {
        return;
    }
    if (tap.type == RemapTarget::Key) {
        input_->key(tap.code, true);
        input_->sync();
        input_->key(tap.code, false);
        input_->sync();
    } else if (tap.type == RemapTarget::MouseButton) {
        input_->click(tap.code, true);
        input_->sync();
        input_->click(tap.code, false);
        input_->sync();
    }
}

void Gamepad::update_tap_hold(const GamepadState& state, const GamepadState& prev) {
    auto now = std::chrono::steady_clock::now();
    const auto* active = get_active_layer();

    for (const auto& [name, layer] : config_.layers) {
        const bool curr = is_button_pressed(state, layer.trigger);
        const bool old = is_button_pressed(prev, layer.trigger);
        const bool released = !curr && old;
        const bool pressed = curr && !old;

        if (active != nullptr && active != &layer) {
            continue;
        }

        if (layer.activation == LayerConfig::Toggle) {
            if (released && toggled_layers_.contains(name)) {
                toggled_layers_.erase(name);
                DBG("Layer '" << name << "' toggled off");
            } else if (released && active == nullptr) {
                tap_hold_states_.clear();
                toggled_layers_.insert(name);
                DBG("Layer '" << name << "' toggled on");
            }
            continue;
        }

        // Hold mode
        if (pressed && active == nullptr) {
            tap_hold_states_[name] = {name, now, false};
            continue;
        }

        auto it = tap_hold_states_.find(name);
        if (it == tap_hold_states_.end()) {
            continue;
        }

        if (released) {
            if (!it->second.layer_activated && layer.tap) {
                emit_tap(*layer.tap);
            }
            tap_hold_states_.erase(it);
            continue;
        }

        if (!it->second.layer_activated && !toggled_layers_.empty()) {
            tap_hold_states_.erase(it);
            continue;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.press_time);
        if (!it->second.layer_activated && elapsed.count() >= layer.hold_timeout) {
            it->second.layer_activated = true;
            DBG("Layer '" << name << "' activated");
        }
    }
}

auto Gamepad::get_effective_gyro() -> const GyroConfig& {
    if (const auto* layer = get_active_layer(); layer != nullptr && layer->gyro) {
        return *layer->gyro;
    }
    return config_.gyro;
}

auto Gamepad::get_effective_stick_left() -> const StickConfig& {
    if (const auto* layer = get_active_layer(); layer != nullptr && layer->stick_left) {
        return *layer->stick_left;
    }
    return config_.left_stick;
}

auto Gamepad::get_effective_stick_right() -> const StickConfig& {
    if (const auto* layer = get_active_layer(); layer != nullptr && layer->stick_right) {
        return *layer->stick_right;
    }
    return config_.right_stick;
}

auto Gamepad::get_effective_dpad() -> const DpadConfig& {
    if (const auto* layer = get_active_layer(); layer != nullptr && layer->dpad) {
        return *layer->dpad;
    }
    return config_.dpad;
}

void Gamepad::process_gyro(const GamepadState& state) {
    const auto& gcfg = get_effective_gyro();

    if (gcfg.mode == GyroConfig::Off) {
        gyro_vel_x_ = gyro_vel_y_ = 0.0F;
        gyro_accum_x_ = gyro_accum_y_ = 0.0F;
        gyro_stick_x_ = gyro_stick_y_ = 0;
        return;
    }

    auto gz = static_cast<float>(-state.gyro_z);
    auto gx = static_cast<float>(-state.gyro_x);
    const auto dz = static_cast<float>(gcfg.deadzone);
    if (std::abs(gz) < dz) {
        gz = 0;
    }
    if (std::abs(gx) < dz) {
        gx = 0;
    }

    gz = apply_curve(gz, gcfg.curve, dz);
    gx = apply_curve(gx, gcfg.curve, dz);

    if (gcfg.mode == GyroConfig::Joystick) {
        constexpr float JOYSTICK_SCALE = 20.0F;
        auto stick_x = gz * gcfg.sensitivity_x * JOYSTICK_SCALE;
        auto stick_y = gx * gcfg.sensitivity_y * JOYSTICK_SCALE;
        if (gcfg.invert_x) {
            stick_x = -stick_x;
        }
        if (gcfg.invert_y) {
            stick_y = -stick_y;
        }
        gyro_stick_x_ = std::clamp(static_cast<int>(stick_x), -AXIS_MAX, AXIS_MAX);
        gyro_stick_y_ = std::clamp(static_cast<int>(stick_y), -AXIS_MAX, AXIS_MAX);
        return;
    }

    if (!input_) {
        return;
    }

    auto raw_x = gz * GYRO_SCALE * gcfg.sensitivity_x;
    auto raw_y = gx * GYRO_SCALE * gcfg.sensitivity_y;

    if (gcfg.invert_x) {
        raw_x = -raw_x;
    }
    if (gcfg.invert_y) {
        raw_y = -raw_y;
    }

    const float smooth = std::clamp(gcfg.smoothing, 0.0F, 0.95F);
    gyro_vel_x_ = (gyro_vel_x_ * smooth) + (raw_x * (1.0F - smooth));
    gyro_vel_y_ = (gyro_vel_y_ * smooth) + (raw_y * (1.0F - smooth));

    gyro_accum_x_ += gyro_vel_x_;
    gyro_accum_y_ += gyro_vel_y_;

    const int dx = static_cast<int>(gyro_accum_x_);
    const int dy = static_cast<int>(gyro_accum_y_);

    if (dx != 0 || dy != 0) {
        gyro_accum_x_ -= static_cast<float>(dx);
        gyro_accum_y_ -= static_cast<float>(dy);
        input_->move_mouse(dx, dy);
        input_->sync();
    }
}

void Gamepad::process_mouse_stick(const GamepadState& state) {
    if (!input_) {
        return;
    }

    const auto& cfg = get_effective_stick_right();
    if (cfg.mode != StickConfig::Mouse) {
        return;
    }

    int rx = state.right_x;
    int ry = state.right_y;
    if (std::abs(rx) < cfg.deadzone) {
        rx = 0;
    }
    if (std::abs(ry) < cfg.deadzone) {
        ry = 0;
    }

    const int dx = static_cast<int>(static_cast<float>(rx) * STICK_SCALE * cfg.sensitivity);
    const int dy = static_cast<int>(static_cast<float>(ry) * STICK_SCALE * cfg.sensitivity);

    if (dx != 0 || dy != 0) {
        input_->move_mouse(dx, dy);
        input_->sync();
    }
}

void Gamepad::process_scroll_stick(const GamepadState& state) {
    if (!input_) {
        return;
    }

    const auto& cfg = get_effective_stick_left();
    if (cfg.mode != StickConfig::Scroll) {
        scroll_accum_v_ = scroll_accum_h_ = 0.0F;
        return;
    }

    int lx = state.left_x;
    int ly = state.left_y;
    if (std::abs(lx) < cfg.deadzone) {
        lx = 0;
    }
    if (std::abs(ly) < cfg.deadzone) {
        ly = 0;
    }

    scroll_accum_v_ += static_cast<float>(-ly) * SCROLL_SCALE * cfg.sensitivity;
    scroll_accum_h_ += static_cast<float>(lx) * SCROLL_SCALE * cfg.sensitivity;

    const int scroll_v = static_cast<int>(scroll_accum_v_);
    const int scroll_h = static_cast<int>(scroll_accum_h_);

    if (scroll_v != 0 || scroll_h != 0) {
        scroll_accum_v_ -= static_cast<float>(scroll_v);
        scroll_accum_h_ -= static_cast<float>(scroll_h);
        input_->scroll(scroll_v, scroll_h);
        input_->sync();
    }
}

void Gamepad::process_layer_dpad(const GamepadState& state) {
    if (!input_) {
        return;
    }

    const auto& cfg = get_effective_dpad();
    const bool active = cfg.mode == DpadConfig::Arrows;

    auto is_up = [](uint8_t dp) {
        return dp == DPAD_UP || dp == DPAD_UP_LEFT || dp == DPAD_UP_RIGHT;
    };
    auto is_down = [](uint8_t dp) {
        return dp == DPAD_DOWN || dp == DPAD_DOWN_LEFT || dp == DPAD_DOWN_RIGHT;
    };
    auto is_left = [](uint8_t dp) {
        return dp == DPAD_LEFT || dp == DPAD_UP_LEFT || dp == DPAD_DOWN_LEFT;
    };
    auto is_right = [](uint8_t dp) {
        return dp == DPAD_RIGHT || dp == DPAD_UP_RIGHT || dp == DPAD_DOWN_RIGHT;
    };

    const bool want_up = active && is_up(state.dpad);
    const bool want_down = active && is_down(state.dpad);
    const bool want_left = active && is_left(state.dpad);
    const bool want_right = active && is_right(state.dpad);

    bool changed = false;
    auto update_key = [&](bool& current, bool want, int code) {
        if (current != want) {
            input_->key(code, want);
            current = want;
            changed = true;
        }
    };

    update_key(dpad_up_, want_up, KEY_UP);
    update_key(dpad_down_, want_down, KEY_DOWN);
    update_key(dpad_left_, want_left, KEY_LEFT);
    update_key(dpad_right_, want_right, KEY_RIGHT);

    if (changed) {
        input_->sync();
    }
}

void Gamepad::process_base_remaps(const GamepadState& state, const GamepadState& prev) {
    if (config_.emulate_elite) {
        return;
    }

    const auto* active_layer = get_active_layer();

    for (const auto& [btn, target] : config_.button_remaps) {
        auto [btn_mask, ext_mask] = button_to_masks(btn);
        suppressed_buttons_ |= btn_mask;
        suppressed_ext_ |= ext_mask;

        if (target.type == RemapTarget::Disabled) {
            continue;
        }

        if (tap_hold_states_.contains(btn)) {
            continue;
        }

        if (active_layer != nullptr && active_layer->remap.contains(btn)) {
            continue;
        }

        if (!input_) {
            continue;
        }

        const bool curr = is_button_pressed(state, btn);
        const bool old = is_button_pressed(prev, btn);
        if (curr == old) {
            continue;
        }

        if (target.type == RemapTarget::Key) {
            input_->key(target.code, curr);
            input_->sync();
        } else if (target.type == RemapTarget::MouseButton) {
            input_->click(target.code, curr);
            input_->sync();
        }
    }
}

void Gamepad::process_layer_buttons(const GamepadState& state, const GamepadState& prev) {
    const auto* layer = get_active_layer();
    if (layer == nullptr) {
        return;
    }

    for (const auto& [btn, target] : layer->remap) {
        auto [btn_mask, ext_mask] = button_to_masks(btn);
        suppressed_buttons_ |= btn_mask;
        suppressed_ext_ |= ext_mask;

        if (target.type == RemapTarget::Disabled || !input_) {
            continue;
        }

        const bool curr = is_button_pressed(state, btn);
        const bool old = is_button_pressed(prev, btn);
        if (curr == old) {
            continue;
        }

        DBG("Layer remap: " << btn << " -> code=" << target.code << " pressed=" << curr);

        if (target.type == RemapTarget::MouseButton) {
            input_->click(target.code, curr);
            input_->sync();
        } else if (target.type == RemapTarget::Key) {
            input_->key(target.code, curr);
            input_->sync();
        }
    }
}

auto Gamepad::poll() -> Result<void> {
    std::array<uint8_t, PKT_SIZE> buf{};
    auto bytes = hidraw_.read(buf);
    if (!bytes) {
        return std::unexpected(bytes.error());
    }

    if (auto state = ext_report::parse({buf.data(), *bytes})) {
        suppressed_buttons_ = 0;
        suppressed_ext_ = 0;

        update_tap_hold(*state, prev_state_);
        process_gyro(*state);
        process_mouse_stick(*state);
        process_scroll_stick(*state);
        process_layer_dpad(*state);
        process_base_remaps(*state, prev_state_);
        process_layer_buttons(*state, prev_state_);

        auto emit_state = *state;
        emit_state.buttons &= ~suppressed_buttons_;
        emit_state.ext_buttons &= ~suppressed_ext_;

        if (get_effective_gyro().mode == GyroConfig::Joystick) {
            emit_state.right_x = static_cast<int16_t>(gyro_stick_x_);
            emit_state.right_y = static_cast<int16_t>(gyro_stick_y_);
        }

        auto emit_prev = prev_state_;
        emit_prev.buttons &= ~prev_suppressed_buttons_;
        emit_prev.ext_buttons &= ~prev_suppressed_ext_;

        auto result = uinput_.emit(emit_state, emit_prev);
        prev_state_ = *state;
        prev_suppressed_buttons_ = suppressed_buttons_;
        prev_suppressed_ext_ = suppressed_ext_;
        return result;
    }
    return {};
}

auto Gamepad::send_rumble(uint8_t left, uint8_t right) -> bool {
    std::array<uint8_t, PKT_SIZE> pkt{};
    pkt.at(0) = MAGIC_5A;
    pkt.at(1) = MAGIC_A5;
    pkt.at(2) = CMD_RUMBLE;
    pkt.at(3) = 0x06;
    pkt.at(4) = left;
    pkt.at(5) = right;
    return hidraw_.write(pkt).has_value();
}

Gamepad::~Gamepad() {
    send_test_mode(hidraw_, false);
}

} // namespace vader5
