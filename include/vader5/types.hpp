#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <system_error>

namespace vader5 {

using Error = std::error_code;

template <typename T> using Result = std::expected<T, Error>;

constexpr uint16_t VENDOR_ID = 0x37d7;
constexpr uint16_t PRODUCT_ID = 0x2401;

// Xbox Elite Series 2 (for uinput emulation)
constexpr uint16_t ELITE_VENDOR_ID = 0x045e;
constexpr uint16_t ELITE_PRODUCT_ID = 0x0b00;

struct GamepadState {
    int16_t left_x{};
    int16_t left_y{};
    int16_t right_x{};
    int16_t right_y{};
    uint8_t left_trigger{};
    uint8_t right_trigger{};

    uint16_t buttons{};
    uint8_t dpad{};
    uint8_t ext_buttons{};
    uint8_t ext_buttons2{};

    int16_t gyro_x{};
    int16_t gyro_y{};
    int16_t gyro_z{};
    int16_t accel_x{};
    int16_t accel_y{};
    int16_t accel_z{};
};

// Gamepad button masks (avoid linux/input.h macro collision)
enum Button : uint16_t {
    PAD_A = 1 << 0,
    PAD_B = 1 << 1,
    PAD_X = 1 << 2,
    PAD_Y = 1 << 3,
    PAD_LB = 1 << 4,
    PAD_RB = 1 << 5,
    PAD_SELECT = 1 << 6,
    PAD_START = 1 << 7,
    PAD_MODE = 1 << 8,
    PAD_L3 = 1 << 9,
    PAD_R3 = 1 << 10,
};

// byte[13] of extended report (Interface 1 test mode)
enum ExtButton : uint8_t {
    EXT_C = 1 << 0,
    EXT_Z = 1 << 1,
    EXT_M1 = 1 << 2,
    EXT_M2 = 1 << 3,
    EXT_M3 = 1 << 4,
    EXT_M4 = 1 << 5,
    EXT_LM = 1 << 6,
    EXT_RM = 1 << 7,
};

// byte[14] of extended report
enum ExtButton2 : uint8_t {
    EXT_O = 1 << 0,
    EXT_HOME = 1 << 3,
};

enum Dpad : uint8_t {
    DPAD_NONE = 0,
    DPAD_UP = 1,
    DPAD_UP_RIGHT = 2,
    DPAD_RIGHT = 3,
    DPAD_DOWN_RIGHT = 4,
    DPAD_DOWN = 5,
    DPAD_DOWN_LEFT = 6,
    DPAD_LEFT = 7,
    DPAD_UP_LEFT = 8,
};

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

} // namespace vader5
