#pragma once

#include "types.hpp"
#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace vader5 {

constexpr size_t PKT_SIZE = 32;
constexpr uint8_t MAGIC_5A = 0x5a;
constexpr uint8_t MAGIC_A5 = 0xa5;
constexpr uint8_t MAGIC_EF = 0xef;

constexpr std::array<uint8_t, 16> DPAD_MAP = {
    DPAD_NONE,       DPAD_UP,   DPAD_RIGHT, DPAD_UP_RIGHT, DPAD_DOWN, DPAD_NONE,
    DPAD_DOWN_RIGHT, DPAD_NONE, DPAD_LEFT,  DPAD_UP_LEFT,  DPAD_NONE, DPAD_NONE,
    DPAD_DOWN_LEFT,  DPAD_NONE, DPAD_NONE,  DPAD_NONE};

inline auto read_s16(const uint8_t* data) -> int16_t {
    return static_cast<int16_t>(static_cast<uint16_t>(data[0]) |
                                (static_cast<uint16_t>(data[1]) << 8));
}

inline auto parse_dpad(uint8_t b11) -> uint8_t {
    return DPAD_MAP[b11 & 0x0F];
}

namespace ext_report {
constexpr size_t OFF_LX = 3;
constexpr size_t OFF_BTNS = 11;
constexpr size_t OFF_EXT1 = 13;
constexpr size_t OFF_EXT2 = 14;
constexpr size_t OFF_LT = 15;
constexpr size_t OFF_RT = 16;
constexpr size_t OFF_GYRO = 17;
constexpr size_t OFF_ACCEL = 23;
constexpr size_t MIN_SIZE = 17;
constexpr size_t FULL_SIZE = 29;

// byte[11]
constexpr uint8_t B11_A = 0x10;
constexpr uint8_t B11_B = 0x20;
constexpr uint8_t B11_SELECT = 0x40;
constexpr uint8_t B11_X = 0x80;

// byte[12]
constexpr uint8_t B12_Y = 0x01;
constexpr uint8_t B12_START = 0x02;
constexpr uint8_t B12_LB = 0x04;
constexpr uint8_t B12_RB = 0x08;
constexpr uint8_t B12_L3 = 0x40;
constexpr uint8_t B12_R3 = 0x80;

inline auto parse_buttons(uint8_t b11, uint8_t b12) -> uint16_t {
    uint16_t btns = 0;
    if ((b11 & B11_A) != 0)
        btns |= PAD_A;
    if ((b11 & B11_B) != 0)
        btns |= PAD_B;
    if ((b11 & B11_X) != 0)
        btns |= PAD_X;
    if ((b12 & B12_Y) != 0)
        btns |= PAD_Y;
    if ((b12 & B12_LB) != 0)
        btns |= PAD_LB;
    if ((b12 & B12_RB) != 0)
        btns |= PAD_RB;
    if ((b11 & B11_SELECT) != 0)
        btns |= PAD_SELECT;
    if ((b12 & B12_START) != 0)
        btns |= PAD_START;
    if ((b12 & B12_L3) != 0)
        btns |= PAD_L3;
    if ((b12 & B12_R3) != 0)
        btns |= PAD_R3;
    return btns;
}

inline auto parse(std::span<const uint8_t> data) -> std::optional<GamepadState> {
    if (data.size() < MIN_SIZE)
        return std::nullopt;
    if (data[0] != MAGIC_5A || data[1] != MAGIC_A5 || data[2] != MAGIC_EF) {
        return std::nullopt;
    }

    GamepadState state{};
    state.left_x = read_s16(&data[OFF_LX]);
    state.left_y = static_cast<int16_t>(-read_s16(&data[OFF_LX + 2]));
    state.right_x = read_s16(&data[OFF_LX + 4]);
    state.right_y = static_cast<int16_t>(-read_s16(&data[OFF_LX + 6]));

    const uint8_t b11 = data[OFF_BTNS];
    const uint8_t b12 = data[OFF_BTNS + 1];
    state.buttons = parse_buttons(b11, b12);
    state.dpad = parse_dpad(b11);
    state.left_trigger = data[OFF_LT];
    state.right_trigger = data[OFF_RT];
    state.ext_buttons = data[OFF_EXT1];
    state.ext_buttons2 = data[OFF_EXT2];

    if (data.size() >= FULL_SIZE) {
        state.gyro_x = read_s16(&data[OFF_GYRO]);
        state.gyro_y = read_s16(&data[OFF_GYRO + 2]);
        state.gyro_z = read_s16(&data[OFF_GYRO + 4]);
        state.accel_x = read_s16(&data[OFF_ACCEL]);
        state.accel_y = read_s16(&data[OFF_ACCEL + 2]);
        state.accel_z = read_s16(&data[OFF_ACCEL + 4]);
    }

    return state;
}
} // namespace ext_report

} // namespace vader5