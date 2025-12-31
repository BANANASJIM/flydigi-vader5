#include "vader5/hidraw.hpp"
#include "vader5/transport.hpp"
#include "vader5/types.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <array>
#include <atomic>
#include <cstdlib>
#include <mutex>
#include <thread>

namespace {
using ftxui::bgcolor;
using ftxui::border;
using ftxui::bold;
using ftxui::center;
using ftxui::CatchEvent;
using ftxui::Color;
using ftxui::color;
using ftxui::dim;
using ftxui::Element;
using ftxui::EQUAL;
using ftxui::Event;
using ftxui::gaugeRight;
using ftxui::hbox;
using ftxui::inverted;
using ftxui::Renderer;
using ftxui::ScreenInteractive;
using ftxui::separator;
using ftxui::size;
using ftxui::text;
using ftxui::vbox;
using ftxui::WIDTH;

constexpr size_t READ_BUFFER_SIZE = 32;
constexpr int READ_TIMEOUT_MS = 16;
constexpr uint8_t IFACE_INPUT = 0;
constexpr int TRIGGER_BAR_WIDTH = 15;
constexpr uint8_t EP_INPUT = 0x01;
constexpr uint8_t EP_OUTPUT = 0x05;
constexpr uint8_t RUMBLE_PKT_SIZE = 8;
constexpr uint8_t RUMBLE_CMD = 0x08;

std::atomic<bool> g_running{true};
vader5::GamepadState g_state{};
std::mutex g_mutex;

std::atomic<uint8_t> g_rumble_left{0};
std::atomic<uint8_t> g_rumble_right{0};
std::atomic<bool> g_rumble_changed{false};

void send_rumble(vader5::UsbTransport& usb, uint8_t left, uint8_t right) {
    std::array<uint8_t, RUMBLE_PKT_SIZE> pkt = {0x00, RUMBLE_CMD, 0x00, left, right, 0x00, 0x00, 0x00};
    auto result = usb.write(pkt, EP_OUTPUT, READ_TIMEOUT_MS);
    (void)result;
}

Element render_stick(const std::string& name, int16_t pos_x, int16_t pos_y, bool pressed) {
    constexpr int RANGE = 32767;
    constexpr int SIZE = 9;
    const int cx = (pos_x + RANGE) * (SIZE - 1) / (2 * RANGE);
    const int cy = (pos_y + RANGE) * (SIZE - 1) / (2 * RANGE);

    std::vector<Element> rows;
    for (int row = 0; row < SIZE; ++row) {
        std::string line;
        for (int col = 0; col < SIZE; ++col) {
            if (row == cy && col == cx) {
                line += "●";
            } else if (row == SIZE / 2 && col == SIZE / 2) {
                line += "┼";
            } else if (row == SIZE / 2) {
                line += "─";
            } else if (col == SIZE / 2) {
                line += "│";
            } else {
                line += " ";
            }
        }
        rows.push_back(text(line));
    }

    auto stick_box = vbox(rows) | border;
    if (pressed) {
        stick_box = stick_box | bgcolor(Color::Blue);
    }

    return vbox({
        text(name) | bold | center,
        stick_box,
        text("X:" + std::to_string(pos_x)) | dim | center,
        text("Y:" + std::to_string(pos_y)) | dim | center,
    });
}

Element render_trigger(const std::string& name, uint8_t value) {
    const float ratio = static_cast<float>(value) / 255.0F;
    return hbox({
        text(name + " ") | bold,
        gaugeRight(ratio) | size(WIDTH, EQUAL, TRIGGER_BAR_WIDTH) | color(Color::Green),
        text(" " + std::to_string(value)),
    });
}

Element render_dpad(uint8_t dpad) {
    auto cell = [&](uint8_t check, const std::string& label) -> Element {
        const bool active = (dpad == check);
        auto elem = text(label) | center;
        return active ? (elem | inverted) : elem;
    };

    return vbox({
        text("D-Pad") | bold | center,
        vbox({
            hbox({text("   "), cell(vader5::DPAD_UP, " ↑ "), text("   ")}),
            hbox({cell(vader5::DPAD_LEFT, " ← "), text("   "), cell(vader5::DPAD_RIGHT, " → ")}),
            hbox({text("   "), cell(vader5::DPAD_DOWN, " ↓ "), text("   ")}),
        }) | border,
    });
}

Element render_face_buttons(uint16_t buttons) {
    auto btn = [&](uint16_t mask, const std::string& label, Color col) -> Element {
        const bool pressed = (buttons & mask) != 0;
        auto elem = text(label) | center;
        if (pressed) {
            return elem | bgcolor(col) | color(Color::White);
        }
        return elem | dim;
    };

    return vbox({
        text("Buttons") | bold | center,
        vbox({
            hbox({text("   "), btn(vader5::BTN_Y, " Y ", Color::Yellow), text("   ")}),
            hbox({btn(vader5::BTN_X, " X ", Color::Blue), text("   "),
                  btn(vader5::BTN_B, " B ", Color::Red)}),
            hbox({text("   "), btn(vader5::BTN_A, " A ", Color::Green), text("   ")}),
        }) | border,
    });
}

Element render_shoulder_buttons(uint16_t buttons, uint8_t lt, uint8_t rt) {
    auto btn = [&](uint16_t mask, const std::string& label) -> Element {
        const bool pressed = (buttons & mask) != 0;
        auto elem = text(label);
        return pressed ? (elem | inverted) : elem;
    };

    return hbox({
        vbox({
            btn(vader5::BTN_LB, " LB "),
            render_trigger("LT", lt),
        }),
        text("     "),
        vbox({
            btn(vader5::BTN_RB, " RB "),
            render_trigger("RT", rt),
        }),
    });
}

Element render_center_buttons(uint16_t buttons) {
    auto btn = [&](uint16_t mask, const std::string& label) -> Element {
        const bool pressed = (buttons & mask) != 0;
        auto elem = text(label);
        return pressed ? (elem | inverted | bold) : (elem | dim);
    };

    return hbox({
        btn(vader5::BTN_SELECT, " SELECT "),
        text("  "),
        btn(vader5::BTN_MODE, " MODE "),
        text("  "),
        btn(vader5::BTN_START, " START "),
    });
}

Element render_rumble(uint8_t left, uint8_t right) {
    auto bar = [](const std::string& name, uint8_t val, Color col) {
        const float ratio = static_cast<float>(val) / 255.0F;
        return hbox({
            text(name + ": "),
            gaugeRight(ratio) | size(WIDTH, EQUAL, TRIGGER_BAR_WIDTH) | color(col),
            text(" " + std::to_string(val)),
        });
    };
    return vbox({
        text("Rumble (X360)") | bold | center,
        bar("L", left, Color::Magenta),
        bar("R", right, Color::Cyan),
    }) | border;
}

void input_thread(vader5::UsbTransport& usb) {
    try {
        std::array<uint8_t, READ_BUFFER_SIZE> buf{};
        while (g_running.load()) {
            auto bytes = usb.read(buf, READ_TIMEOUT_MS);
            if (bytes && *bytes > 0) {
                auto state = vader5::Hidraw::parse_report({buf.data(), *bytes});
                if (state) {
                    const std::scoped_lock lock(g_mutex);
                    g_state = *state;
                }
            }
            if (g_rumble_changed.exchange(false)) {
                send_rumble(usb, g_rumble_left.load(), g_rumble_right.load());
            }
        }
        send_rumble(usb, 0, 0);
    } catch (...) {
        g_running.store(false);
    }
}
} // namespace

auto main() -> int {
    auto usb = vader5::UsbTransport::open(vader5::VENDOR_ID, vader5::PRODUCT_ID,
                                          IFACE_INPUT, EP_INPUT);
    if (!usb) {
        return EXIT_FAILURE;
    }

    std::thread reader(input_thread, std::ref(*usb));
    auto screen = ScreenInteractive::Fullscreen();

    auto renderer = Renderer([&] {
        vader5::GamepadState st;
        {
            const std::scoped_lock lock(g_mutex);
            st = g_state;
        }

        const bool l3 = (st.buttons & vader5::BTN_L3) != 0;
        const bool r3 = (st.buttons & vader5::BTN_R3) != 0;
        const uint8_t rumble_l = g_rumble_left.load();
        const uint8_t rumble_r = g_rumble_right.load();

        return vbox({
            text("═══ Vader 5 Pro Debug ═══") | bold | center,
            text(""),
            render_shoulder_buttons(st.buttons, st.left_trigger, st.right_trigger) | center,
            text(""),
            render_center_buttons(st.buttons) | center,
            text(""),
            hbox({
                render_stick("L Stick", st.left_x, st.left_y, l3),
                text("    "),
                render_dpad(st.dpad),
                text("    "),
                render_face_buttons(st.buttons),
                text("    "),
                render_stick("R Stick", st.right_x, st.right_y, r3),
            }) | center,
            text(""),
            separator(),
            render_rumble(rumble_l, rumble_r) | center,
            text("[1-9] intensity  [Z] left  [X] right  [C] both  [0] stop") | dim | center,
            separator(),
            text("Press Q to quit") | dim | center,
        }) | border | center;
    });

    auto component = CatchEvent(renderer, [&](const Event& event) {
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            g_running.store(false);
            screen.Exit();
            return true;
        }
        constexpr uint8_t INTENSITY_STEP = 28;
        if (event.character().size() == 1) {
            const char ch = event.character()[0];
            if (ch >= '1' && ch <= '9') {
                auto intensity = static_cast<uint8_t>((ch - '0') * INTENSITY_STEP);
                g_rumble_left.store(intensity);
                g_rumble_right.store(intensity);
                g_rumble_changed.store(true);
                return true;
            }
            if (ch == '0') {
                g_rumble_left.store(0);
                g_rumble_right.store(0);
                g_rumble_changed.store(true);
                return true;
            }
            if (ch == 'z' || ch == 'Z') {
                g_rumble_left.store(255);
                g_rumble_right.store(0);
                g_rumble_changed.store(true);
                return true;
            }
            if (ch == 'x' || ch == 'X') {
                g_rumble_left.store(0);
                g_rumble_right.store(255);
                g_rumble_changed.store(true);
                return true;
            }
            if (ch == 'c' || ch == 'C') {
                g_rumble_left.store(255);
                g_rumble_right.store(255);
                g_rumble_changed.store(true);
                return true;
            }
        }
        return false;
    });

    std::thread refresh([&] {
        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            screen.PostEvent(Event::Custom);
        }
    });

    screen.Loop(component);

    g_running.store(false);
    refresh.join();
    reader.join();
    return 0;
}