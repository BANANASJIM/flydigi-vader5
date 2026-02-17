#include "vader5/config.hpp"
#include "vader5/types.hpp"

#include <cstdlib>
#include <iostream>

using namespace vader5;

#define CHECK(expr)                                                                                \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            std::cerr << "FAIL: " #expr " (" << __FILE__ << ":" << __LINE__ << ")\n";              \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (0)

void test_parse_gamepad_buttons() {
    auto l3 = parse_remap_target("L3");
    CHECK(l3 && l3->type == RemapTarget::GamepadButton);
    CHECK(l3->btn_mask == PAD_L3 && l3->ext_mask == 0);

    auto r3 = parse_remap_target("R3");
    CHECK(r3 && r3->type == RemapTarget::GamepadButton && r3->btn_mask == PAD_R3);

    auto a = parse_remap_target("A");
    CHECK(a && a->type == RemapTarget::GamepadButton && a->btn_mask == PAD_A);

    auto lm = parse_remap_target("LM");
    CHECK(lm && lm->type == RemapTarget::GamepadButton);
    CHECK(lm->ext_mask == EXT_LM && lm->btn_mask == 0);

    auto rm = parse_remap_target("RM");
    CHECK(rm && rm->type == RemapTarget::GamepadButton && rm->ext_mask == EXT_RM);

    auto m1 = parse_remap_target("M1");
    CHECK(m1 && m1->type == RemapTarget::GamepadButton && m1->ext_mask == EXT_M1);

    std::cout << "  parse gamepad buttons: OK\n";
}

void test_parse_existing_types() {
    auto key = parse_remap_target("KEY_A");
    CHECK(key && key->type == RemapTarget::Key);

    auto mouse = parse_remap_target("mouse_left");
    CHECK(mouse && mouse->type == RemapTarget::MouseButton);

    auto disabled = parse_remap_target("disabled");
    CHECK(disabled && disabled->type == RemapTarget::Disabled);

    CHECK(!parse_remap_target("invalid_junk"));

    std::cout << "  parse existing types: OK\n";
}

void test_injection_lm_to_l3() {
    GamepadState state{};
    state.ext_buttons = EXT_LM;

    auto [src_btn, src_ext] = button_to_masks("LM");
    auto target = *parse_remap_target("L3");

    uint16_t injected_btn = 0;
    uint8_t injected_ext = 0;

    if ((state.ext_buttons & EXT_LM) != 0) {
        injected_btn |= target.btn_mask;
        injected_ext |= target.ext_mask;
    }

    auto emit = state;
    emit.buttons = (emit.buttons & ~src_btn) | injected_btn;
    emit.ext_buttons = (emit.ext_buttons & ~src_ext) | injected_ext;

    CHECK((emit.ext_buttons & EXT_LM) == 0);
    CHECK((emit.buttons & PAD_L3) != 0);
    std::cout << "  LM -> L3 injection: OK\n";
}

void test_injection_button_swap() {
    GamepadState state{};
    state.buttons = PAD_B;

    auto target_a = *parse_remap_target("A");
    auto [src_btn, src_ext] = button_to_masks("B");
    (void)src_ext;

    uint16_t injected = 0;
    if ((state.buttons & PAD_B) != 0) {
        injected |= target_a.btn_mask;
    }

    auto emit = state;
    emit.buttons = (emit.buttons & ~src_btn) | injected;

    CHECK((emit.buttons & PAD_B) == 0);
    CHECK((emit.buttons & PAD_A) != 0);
    std::cout << "  B -> A swap injection: OK\n";
}

void test_no_injection_when_released() {
    GamepadState state{};

    uint16_t injected = 0;
    auto target = *parse_remap_target("L3");

    if ((state.ext_buttons & EXT_LM) != 0) {
        injected |= target.btn_mask;
    }

    auto emit = state;
    emit.buttons |= injected;

    CHECK((emit.buttons & PAD_L3) == 0);
    std::cout << "  no injection when released: OK\n";
}

void test_config_load() {
    auto cfg = Config::load("config/test-gamepad-remap.toml");
    CHECK(cfg.has_value());

    CHECK(cfg->button_remaps.contains("LM"));
    CHECK(cfg->button_remaps.at("LM").type == RemapTarget::GamepadButton);
    CHECK(cfg->button_remaps.at("LM").btn_mask == PAD_L3);

    CHECK(cfg->button_remaps.contains("RM"));
    CHECK(cfg->button_remaps.at("RM").type == RemapTarget::GamepadButton);
    CHECK(cfg->button_remaps.at("RM").btn_mask == PAD_R3);

    std::cout << "  config load (simple): OK\n";
}

void test_config_load_full() {
    auto cfg = Config::load("config/test-remap.toml");
    CHECK(cfg.has_value());

    // Base remap: LM -> L3, RM -> R3
    CHECK(cfg->button_remaps.at("LM").type == RemapTarget::GamepadButton);
    CHECK(cfg->button_remaps.at("LM").btn_mask == PAD_L3);
    CHECK(cfg->button_remaps.at("RM").btn_mask == PAD_R3);

    // Keyboard remaps still work
    CHECK(cfg->button_remaps.at("M1").type == RemapTarget::Key);
    CHECK(cfg->button_remaps.at("A").type == RemapTarget::Key);

    // Mouse remaps still work
    CHECK(cfg->button_remaps.at("M3").type == RemapTarget::MouseButton);

    // Disabled still works
    CHECK(cfg->button_remaps.at("C").type == RemapTarget::Disabled);

    // Layer with gamepad button remaps
    CHECK(cfg->layers.contains("gamepad"));
    const auto& gl = cfg->layers.at("gamepad");
    CHECK(gl.remap.at("A").type == RemapTarget::GamepadButton);
    CHECK(gl.remap.at("A").btn_mask == PAD_B);
    CHECK(gl.remap.at("B").btn_mask == PAD_A);
    CHECK(gl.remap.at("M3").ext_mask == 0);
    CHECK(gl.remap.at("M3").btn_mask == PAD_L3);
    CHECK(gl.remap.at("M4").btn_mask == PAD_R3);

    std::cout << "  config load (full): OK\n";
}

int main() {
    std::cout << "Running gamepad button remap tests...\n";
    test_parse_gamepad_buttons();
    test_parse_existing_types();
    test_injection_lm_to_l3();
    test_injection_button_swap();
    test_no_injection_when_released();
    test_config_load();
    test_config_load_full();
    std::cout << "All tests passed!\n";
}
