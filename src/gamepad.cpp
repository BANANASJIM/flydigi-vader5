#include "vader5/gamepad.hpp"
#include "vader5/protocol.hpp"

#include <linux/input-event-codes.h>

#include <array>
#include <cmath>
#include <print>
#include <thread>

namespace vader5 {

namespace {
constexpr int CONFIG_INTERFACE = 1;
constexpr float GYRO_SCALE = 0.001f;
constexpr float STICK_SCALE = 0.0001f;
constexpr float GYRO_MAX = 32768.0f;

// Apply power curve: faster movements get amplified more when curve > 1
// Normalizes within active range (after deadzone) to avoid compression at low values
auto apply_curve(float value, float curve, float deadzone = 0.0f) -> float {
    if (curve == 1.0f) return value;
    float abs_val = std::abs(value);
    if (abs_val <= deadzone) return value;

    float range = GYRO_MAX - deadzone;
    float normalized = std::clamp((abs_val - deadzone) / range, 0.0f, 1.0f);
    float curved = std::pow(normalized, curve);
    float result = curved * range + deadzone;
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
    if (!hid.write(pkt)) return false;

    std::array<uint8_t, PKT_SIZE> resp{};
    for (int retry = 0; retry < 10; ++retry) {
        auto result = hid.read(resp);
        if (result && *result >= 4 && resp[0] == MAGIC_5A && resp[1] == MAGIC_A5) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
}

auto drain_buffer(Hidraw& hid) -> void {
    std::array<uint8_t, PKT_SIZE> buf{};
    for (int count = 0; count < 10 && hid.read(buf).value_or(0) > 0; ++count) {}
}

auto send_init(Hidraw& hid) -> bool {
    drain_buffer(hid);
    return send_cmd(hid, CMD_01) && send_cmd(hid, CMD_A1) &&
           send_cmd(hid, CMD_02) && send_cmd(hid, CMD_04);
}

auto send_test_mode(Hidraw& hid, bool enable) -> bool {
    std::array<uint8_t, PKT_SIZE> pkt{};
    pkt[0] = MAGIC_5A;
    pkt[1] = MAGIC_A5;
    pkt[2] = CMD_TEST_MODE;
    pkt[3] = 0x07;
    pkt[4] = 0xff;
    pkt[5] = enable ? 0x01 : 0x00;
    pkt[6] = 0xff;
    pkt[7] = 0xff;
    pkt[8] = 0xff;
    pkt[9] = enable ? CHECKSUM_TEST_ON : CHECKSUM_TEST_OFF;
    return hid.write(pkt).has_value();
}

auto needs_mouse(const Config& cfg) -> bool {
    if (cfg.gyro.mode == GyroConfig::Mouse) return true;
    if (cfg.left_stick.as_mouse || cfg.right_stick.as_mouse) return true;
    for (const auto& [_, ms] : cfg.mode_shifts) {
        if (ms.gyro == GyroConfig::Mouse || ms.right_stick_mouse) return true;
        if (ms.left_stick_scroll || ms.dpad_arrows) return true;
        for (const auto& [__, target] : ms.remaps) {
            if (target.type == RemapTarget::MouseButton) return true;
        }
    }
    return false;
}
} // namespace

auto Gamepad::open(const Config& cfg) -> Result<Gamepad> {
    auto hid = Hidraw::open(VENDOR_ID, PRODUCT_ID, CONFIG_INTERFACE);
    if (!hid) return std::unexpected(hid.error());

    if (!send_init(*hid) || !send_test_mode(*hid, true)) {
        return std::unexpected(std::make_error_code(std::errc::protocol_error));
    }

    auto uinput = Uinput::create(cfg.ext_mappings);
    if (!uinput) return std::unexpected(uinput.error());

    std::optional<InputDevice> input;
    if (needs_mouse(cfg)) {
        auto dev = InputDevice::create();
        if (!dev) return std::unexpected(dev.error());
        input = std::move(*dev);
    }

    return Gamepad(std::move(*hid), std::move(*uinput), std::move(input), cfg);
}

auto Gamepad::is_button_pressed(const GamepadState& state, std::string_view name) -> bool {
    if (name == "LM") return (state.ext_buttons & EXT_LM) != 0;
    if (name == "RM") return (state.ext_buttons & EXT_RM) != 0;
    if (name == "C") return (state.ext_buttons & EXT_C) != 0;
    if (name == "Z") return (state.ext_buttons & EXT_Z) != 0;
    if (name == "M1") return (state.ext_buttons & EXT_M1) != 0;
    if (name == "M2") return (state.ext_buttons & EXT_M2) != 0;
    if (name == "M3") return (state.ext_buttons & EXT_M3) != 0;
    if (name == "M4") return (state.ext_buttons & EXT_M4) != 0;
    if (name == "A") return (state.buttons & PAD_A) != 0;
    if (name == "B") return (state.buttons & PAD_B) != 0;
    if (name == "X") return (state.buttons & PAD_X) != 0;
    if (name == "Y") return (state.buttons & PAD_Y) != 0;
    if (name == "RB") return (state.buttons & PAD_RB) != 0;
    if (name == "LB") return (state.buttons & PAD_LB) != 0;
    if (name == "START") return (state.buttons & PAD_START) != 0;
    if (name == "SELECT") return (state.buttons & PAD_SELECT) != 0;
    if (name == "L3") return (state.buttons & PAD_L3) != 0;
    if (name == "R3") return (state.buttons & PAD_R3) != 0;
    if (name == "RT") return state.right_trigger > 128;
    if (name == "LT") return state.left_trigger > 128;
    return false;
}

auto Gamepad::get_active_mode_shift(const GamepadState& state) -> const ModeShiftConfig* {
    for (const auto& [trigger, ms] : config_.mode_shifts) {
        if (is_button_pressed(state, trigger)) return &ms;
    }
    return nullptr;
}

void Gamepad::process_gyro(const GamepadState& state) {
    if (!input_) return;

    const auto* ms = get_active_mode_shift(state);
    GyroConfig::Mode mode = config_.gyro.mode;
    if (ms != nullptr && ms->gyro != GyroConfig::Off) mode = ms->gyro;
    if (mode != GyroConfig::Mouse) {
        gyro_vel_x_ = gyro_vel_y_ = 0.0f;
        gyro_accum_x_ = gyro_accum_y_ = 0.0f;
        return;
    }

    const auto& gcfg = config_.gyro;

    float gz = static_cast<float>(-state.gyro_z);
    float gx = static_cast<float>(-state.gyro_x);
    if (std::abs(gz) < static_cast<float>(gcfg.deadzone)) gz = 0;
    if (std::abs(gx) < static_cast<float>(gcfg.deadzone)) gx = 0;

    // Apply response curve: curve > 1 amplifies fast movements
    float dz = static_cast<float>(gcfg.deadzone);
    gz = apply_curve(gz, gcfg.curve, dz);
    gx = apply_curve(gx, gcfg.curve, dz);

    float raw_x = gz * GYRO_SCALE * gcfg.sensitivity_x;
    float raw_y = gx * GYRO_SCALE * gcfg.sensitivity_y;

    if (gcfg.invert_x) raw_x = -raw_x;
    if (gcfg.invert_y) raw_y = -raw_y;

    // Smooth velocity to filter hand shake
    float smooth = std::clamp(gcfg.smoothing, 0.0f, 0.95f);
    gyro_vel_x_ = gyro_vel_x_ * smooth + raw_x * (1.0f - smooth);
    gyro_vel_y_ = gyro_vel_y_ * smooth + raw_y * (1.0f - smooth);

    // Sub-pixel accumulation - keep fractional movement for next frame
    gyro_accum_x_ += gyro_vel_x_;
    gyro_accum_y_ += gyro_vel_y_;

    int dx = static_cast<int>(gyro_accum_x_);
    int dy = static_cast<int>(gyro_accum_y_);

    if (dx != 0 || dy != 0) {
        gyro_accum_x_ -= static_cast<float>(dx);
        gyro_accum_y_ -= static_cast<float>(dy);
        input_->move_mouse(dx, dy);
        input_->sync();
    }
}

void Gamepad::process_mouse_stick(const GamepadState& state) {
    if (!input_) return;

    const auto* ms = get_active_mode_shift(state);
    bool right_mouse = config_.right_stick.as_mouse;
    if (ms != nullptr) right_mouse = ms->right_stick_mouse;
    if (!right_mouse) return;

    float sens = config_.right_stick.mouse_sensitivity;
    int dz = config_.right_stick.deadzone;

    int rx = state.right_x;
    int ry = state.right_y;
    if (std::abs(rx) < dz) rx = 0;
    if (std::abs(ry) < dz) ry = 0;

    int dx = static_cast<int>(static_cast<float>(rx) * STICK_SCALE * sens);
    int dy = static_cast<int>(static_cast<float>(ry) * STICK_SCALE * sens);

    if (dx != 0 || dy != 0) {
        input_->move_mouse(dx, dy);
        input_->sync();
    }
}

void Gamepad::process_mode_shift_buttons(const GamepadState& state, const GamepadState& prev) {
    const auto* ms = get_active_mode_shift(state);
    if (ms == nullptr || !input_) return;

    constexpr std::array<std::string_view, 18> ALL_BUTTONS = {
        "A", "B", "X", "Y", "RB", "LB", "START", "SELECT", "L3", "R3",
        "RT", "LT", "C", "Z", "M1", "M2", "M3", "M4"
    };

    for (auto name : ALL_BUTTONS) {
        bool curr = is_button_pressed(state, name);
        bool old = is_button_pressed(prev, name);
        if (curr == old) continue;

        auto it = ms->remaps.find(std::string(name));
        if (it == ms->remaps.end()) continue;

        const auto& target = it->second;
        if (target.type == RemapTarget::MouseButton) {
            input_->click(target.code, curr);
            input_->sync();
        } else if (target.type == RemapTarget::Key) {
            input_->key(target.code, curr);
            input_->sync();
        }
    }
}

void Gamepad::process_scroll_stick(const GamepadState& state) {
    if (!input_) return;

    const auto* ms = get_active_mode_shift(state);
    if (ms == nullptr || !ms->left_stick_scroll) {
        scroll_accum_v_ = scroll_accum_h_ = 0.0f;
        return;
    }

    int dz = config_.left_stick.deadzone;
    int lx = state.left_x;
    int ly = state.left_y;
    if (std::abs(lx) < dz) lx = 0;
    if (std::abs(ly) < dz) ly = 0;

    constexpr float SCROLL_SCALE = 0.00005f;
    float sens = ms->scroll_sensitivity;
    scroll_accum_v_ += static_cast<float>(-ly) * SCROLL_SCALE * sens;
    scroll_accum_h_ += static_cast<float>(lx) * SCROLL_SCALE * sens;

    int scroll_v = static_cast<int>(scroll_accum_v_);
    int scroll_h = static_cast<int>(scroll_accum_h_);

    if (scroll_v != 0 || scroll_h != 0) {
        scroll_accum_v_ -= static_cast<float>(scroll_v);
        scroll_accum_h_ -= static_cast<float>(scroll_h);
        input_->scroll(scroll_v, scroll_h);
        input_->sync();
    }
}

void Gamepad::process_mode_shift_dpad(const GamepadState& state, const GamepadState& /*prev*/) {
    if (!input_) return;

    const auto* ms = get_active_mode_shift(state);
    bool active = ms != nullptr && ms->dpad_arrows;

    auto is_up = [](uint8_t d) { return d == DPAD_UP || d == DPAD_UP_LEFT || d == DPAD_UP_RIGHT; };
    auto is_down = [](uint8_t d) { return d == DPAD_DOWN || d == DPAD_DOWN_LEFT || d == DPAD_DOWN_RIGHT; };
    auto is_left = [](uint8_t d) { return d == DPAD_LEFT || d == DPAD_UP_LEFT || d == DPAD_DOWN_LEFT; };
    auto is_right = [](uint8_t d) { return d == DPAD_RIGHT || d == DPAD_UP_RIGHT || d == DPAD_DOWN_RIGHT; };

    bool want_up = active && is_up(state.dpad);
    bool want_down = active && is_down(state.dpad);
    bool want_left = active && is_left(state.dpad);
    bool want_right = active && is_right(state.dpad);

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

    if (changed) input_->sync();
}

auto Gamepad::poll() -> Result<void> {
    std::array<uint8_t, PKT_SIZE> buf{};
    auto bytes = hidraw_.read(buf);
    if (!bytes) return std::unexpected(bytes.error());

    if (auto state = ext_report::parse({buf.data(), *bytes})) {
        process_gyro(*state);
        process_mouse_stick(*state);
        process_scroll_stick(*state);
        process_mode_shift_dpad(*state, prev_state_);
        process_mode_shift_buttons(*state, prev_state_);

        auto result = uinput_.emit(*state, prev_state_);
        prev_state_ = *state;
        return result;
    }
    return {};
}

// Rumble command - sets motor intensity
// Format: 5a a5 12 06 [left] [right] 00...
auto Gamepad::send_rumble(uint8_t left, uint8_t right) -> bool {
    std::array<uint8_t, PKT_SIZE> pkt{};
    pkt[0] = MAGIC_5A;
    pkt[1] = MAGIC_A5;
    pkt[2] = CMD_RUMBLE;
    pkt[3] = 0x06;  // payload length
    pkt[4] = left;  // left motor (0-255)
    pkt[5] = right; // right motor (0-255)
    return hidraw_.write(pkt).has_value();
}

Gamepad::~Gamepad() {
    send_test_mode(hidraw_, false);
}

} // namespace vader5