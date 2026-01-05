#include "vader5/keycodes.hpp"

#include <array>

namespace vader5 {

namespace {
struct KeyEntry {
    std::string_view name;
    int code;
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
constexpr std::array KEY_TABLE = {
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
    KeyEntry{"KEY_LEFTMETA", KEY_LEFTMETA},
    KeyEntry{"KEY_RIGHTMETA", KEY_RIGHTMETA},
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
    KeyEntry{"BTN_LEFT", BTN_LEFT},
    KeyEntry{"BTN_RIGHT", BTN_RIGHT},
    KeyEntry{"BTN_MIDDLE", BTN_MIDDLE},
    KeyEntry{"BTN_SIDE", BTN_SIDE},
    KeyEntry{"BTN_EXTRA", BTN_EXTRA},
    KeyEntry{"BTN_FORWARD", BTN_FORWARD},
    KeyEntry{"BTN_BACK", BTN_BACK},
};
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
} // namespace

auto keycode_from_name(std::string_view name) -> std::optional<int> {
    for (const auto& entry : KEY_TABLE) {
        if (entry.name == name) {
            return entry.code;
        }
    }
    return std::nullopt;
}

} // namespace vader5