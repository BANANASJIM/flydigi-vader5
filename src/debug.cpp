#include "vader5/hidraw.hpp"
#include "vader5/types.hpp"

#include <poll.h>

#include <array>
#include <cstdlib>
#include <print>

namespace {
constexpr size_t READ_BUFFER_SIZE = 64;
constexpr int POLL_TIMEOUT_MS = 100;
constexpr int GAMEPAD_INTERFACE = 2;

void print_state(const vader5::GamepadState& st) {
    std::print("\033[2J\033[H");
    std::println("=== Vader 5 Pro Debug ===\n");

    std::println("Sticks:");
    std::println("  Left:  X={:6}  Y={:6}", st.left_x, st.left_y);
    std::println("  Right: X={:6}  Y={:6}\n", st.right_x, st.right_y);

    std::println("Triggers:");
    std::println("  LT={:3}  RT={:3}\n", st.left_trigger, st.right_trigger);

    std::print("D-Pad: ");
    switch (st.dpad) {
    case vader5::DPAD_UP:
        std::print("UP");
        break;
    case vader5::DPAD_UP_RIGHT:
        std::print("UP+RIGHT");
        break;
    case vader5::DPAD_RIGHT:
        std::print("RIGHT");
        break;
    case vader5::DPAD_DOWN_RIGHT:
        std::print("DOWN+RIGHT");
        break;
    case vader5::DPAD_DOWN:
        std::print("DOWN");
        break;
    case vader5::DPAD_DOWN_LEFT:
        std::print("DOWN+LEFT");
        break;
    case vader5::DPAD_LEFT:
        std::print("LEFT");
        break;
    case vader5::DPAD_UP_LEFT:
        std::print("UP+LEFT");
        break;
    default:
        std::print("-");
        break;
    }
    std::println("\n");

    std::print("Buttons: ");
    if ((st.buttons & vader5::BTN_A) != 0) {
        std::print("A ");
    }
    if ((st.buttons & vader5::BTN_B) != 0) {
        std::print("B ");
    }
    if ((st.buttons & vader5::BTN_X) != 0) {
        std::print("X ");
    }
    if ((st.buttons & vader5::BTN_Y) != 0) {
        std::print("Y ");
    }
    if ((st.buttons & vader5::BTN_LB) != 0) {
        std::print("LB ");
    }
    if ((st.buttons & vader5::BTN_RB) != 0) {
        std::print("RB ");
    }
    if ((st.buttons & vader5::BTN_SELECT) != 0) {
        std::print("SELECT ");
    }
    if ((st.buttons & vader5::BTN_START) != 0) {
        std::print("START ");
    }
    if ((st.buttons & vader5::BTN_MODE) != 0) {
        std::print("MODE ");
    }
    if ((st.buttons & vader5::BTN_L3) != 0) {
        std::print("L3 ");
    }
    if ((st.buttons & vader5::BTN_R3) != 0) {
        std::print("R3 ");
    }
    std::println("\n");

    std::print("Extended: ");
    if ((st.ext_buttons & vader5::EXT_C) != 0) {
        std::print("C ");
    }
    if ((st.ext_buttons & vader5::EXT_Z) != 0) {
        std::print("Z ");
    }
    if ((st.ext_buttons & vader5::EXT_M1) != 0) {
        std::print("M1 ");
    }
    if ((st.ext_buttons & vader5::EXT_M2) != 0) {
        std::print("M2 ");
    }
    if ((st.ext_buttons & vader5::EXT_M3) != 0) {
        std::print("M3 ");
    }
    if ((st.ext_buttons & vader5::EXT_M4) != 0) {
        std::print("M4 ");
    }
    if ((st.ext_buttons & vader5::EXT_CIRCLE) != 0) {
        std::print("CIRCLE ");
    }
    if ((st.ext_buttons & vader5::EXT_HOME) != 0) {
        std::print("HOME ");
    }
    std::println("\n");

    std::println("Press Ctrl+C to exit");
}
} // namespace

auto main() -> int {
    auto hid = vader5::Hidraw::open(vader5::VENDOR_ID, vader5::PRODUCT_ID, GAMEPAD_INTERFACE);
    if (!hid) {
        std::println(stderr, "Failed to open device: {}", hid.error().message());
        return EXIT_FAILURE;
    }

    std::println("Device opened. Reading input...");

    pollfd pfd{.fd = hid->fd(), .events = POLLIN, .revents = 0};
    std::array<uint8_t, READ_BUFFER_SIZE> buf{};

    while (true) {
        if (poll(&pfd, 1, POLL_TIMEOUT_MS) > 0) {
            auto bytes = hid->read(buf);
            if (bytes && *bytes > 0) {
                auto state = vader5::Hidraw::parse_report({buf.data(), *bytes});
                if (state) {
                    print_state(*state);
                }
            }
        }
    }
}
