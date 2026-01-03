#include "vader5/hidraw.hpp"
#include "vader5/types.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <optional>
#include <span>
#include <thread>

namespace {
using ftxui::bgcolor;
using ftxui::bold;
using ftxui::border;
using ftxui::CatchEvent;
using ftxui::center;
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
constexpr uint8_t IFACE_CONFIG = 1;
constexpr int TRIGGER_BAR_WIDTH = 15;

constexpr uint8_t MAGIC_5A = 0x5a;
constexpr uint8_t MAGIC_A5 = 0xa5;
constexpr uint8_t CMD_TEST_MODE = 0x11;
constexpr uint8_t CMD_TEST_LEN = 0x07;
constexpr uint8_t MODE_TEST = 0x15;
constexpr uint8_t MODE_NORMAL = 0x14;

std::atomic<bool> g_running{true};
vader5::GamepadState g_state{};
std::mutex g_mutex;

std::atomic<bool> g_test_mode{false};
std::atomic<bool> g_test_mode_changed{false};
std::atomic<uint8_t> g_ext_buttons{0};
std::atomic<uint8_t> g_ext_buttons2{0};

constexpr size_t MAX_LOG_LINES = 5;
std::vector<std::string> g_logs;
std::mutex g_log_mutex;

void add_log(const std::string& msg) {
    const std::scoped_lock lock(g_log_mutex);
    g_logs.push_back(msg);
    if (g_logs.size() > MAX_LOG_LINES) {
        g_logs.erase(g_logs.begin());
    }
}

struct ImuData {
    int16_t gyro_x{}, gyro_y{}, gyro_z{};
    int16_t accel_x{}, accel_y{}, accel_z{};
};
ImuData g_imu{};
std::mutex g_imu_mutex;

constexpr size_t CFG_PKT_SIZE = 32;

void send_cmd(vader5::Hidraw& hidraw, std::span<const uint8_t> data) {
    std::array<uint8_t, CFG_PKT_SIZE> pkt{};
    std::ranges::copy(data, pkt.begin());

    auto write_result = hidraw.write(pkt);
    if (!write_result) {
        add_log("CMD " + std::to_string(data[2]) + " write FAIL");
        return;
    }

    std::array<uint8_t, CFG_PKT_SIZE> resp{};
    constexpr int MAX_RETRIES = 10;
    for (int i = 0; i < MAX_RETRIES; ++i) {
        auto read_result = hidraw.read(resp);
        if (read_result && *read_result >= 4 && resp[0] == MAGIC_5A && resp[1] == MAGIC_A5) {
            add_log("CMD " + std::to_string(data[2]) + " → " + std::to_string(resp[2]));
            return;
        }
    }
    add_log("CMD " + std::to_string(data[2]) + " no resp");
}

void drain_ep2(vader5::Hidraw& hidraw) {
    std::array<uint8_t, CFG_PKT_SIZE> buf{};
    int drained = 0;
    while (true) {
        auto result = hidraw.read(buf);
        if (!result || *result == 0) {
            break;
        }
        ++drained;
        if (drained > 10) {
            break;
        }
    }
    if (drained > 0) {
        add_log("Drained " + std::to_string(drained) + " pending packets");
    }
}

void send_init_sequence(vader5::Hidraw& hidraw_cfg) {
    drain_ep2(hidraw_cfg);

    constexpr std::array<uint8_t, 5> CMD_01 = {0x5a, 0xa5, 0x01, 0x02, 0x03};
    constexpr std::array<uint8_t, 5> CMD_A1 = {0x5a, 0xa5, 0xa1, 0x02, 0xa3};
    constexpr std::array<uint8_t, 5> CMD_02 = {0x5a, 0xa5, 0x02, 0x02, 0x04};
    constexpr std::array<uint8_t, 5> CMD_04 = {0x5a, 0xa5, 0x04, 0x02, 0x06};

    add_log("Init: sending handshake...");
    send_cmd(hidraw_cfg, CMD_01);
    send_cmd(hidraw_cfg, CMD_A1);
    send_cmd(hidraw_cfg, CMD_02);
    send_cmd(hidraw_cfg, CMD_04);
    add_log("Init: handshake done");
}

void send_test_mode(vader5::Hidraw& hidraw_cfg, bool enable) {
    std::array<uint8_t, CFG_PKT_SIZE> pkt{};
    pkt.at(0) = MAGIC_5A;
    pkt.at(1) = MAGIC_A5;
    pkt.at(2) = CMD_TEST_MODE;
    pkt.at(3) = CMD_TEST_LEN;
    pkt.at(4) = 0xff;
    pkt.at(5) = enable ? 0x01 : 0x00;
    pkt.at(6) = 0xff;
    pkt.at(7) = 0xff;
    pkt.at(8) = 0xff;
    pkt.at(9) = enable ? MODE_TEST : MODE_NORMAL;
    auto result = hidraw_cfg.write(pkt);
    if (result) {
        add_log(std::string("Test mode ") + (enable ? "ON" : "OFF") + ": " +
                std::to_string(*result) + "B");
    } else {
        add_log(std::string("Test mode ") + (enable ? "ON" : "OFF") + ": FAILED");
    }
}

Element render_ext_buttons(uint8_t ext1, uint8_t ext2, bool test_mode) {
    auto btn = [](bool pressed, const std::string& label) -> Element {
        auto elem = text(label);
        return pressed ? (elem | inverted | bold) : (elem | dim);
    };

    if (!test_mode) {
        return hbox({
            text("Ext: ") | dim,
            text("[T] to enable test mode") | dim,
        });
    }

    return hbox({
        text("Ext: "),
        btn((ext1 & vader5::EXT_C) != 0, " C "),
        btn((ext1 & vader5::EXT_Z) != 0, " Z "),
        btn((ext1 & vader5::EXT_M1) != 0, " M1 "),
        btn((ext1 & vader5::EXT_M2) != 0, " M2 "),
        btn((ext1 & vader5::EXT_M3) != 0, " M3 "),
        btn((ext1 & vader5::EXT_M4) != 0, " M4 "),
        btn((ext1 & vader5::EXT_LM) != 0, " LM "),
        btn((ext1 & vader5::EXT_RM) != 0, " RM "),
        btn((ext2 & vader5::EXT_O) != 0, " O "),
        btn((ext2 & vader5::EXT_HOME) != 0, " Home "),
    });
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
                line += "● ";
            } else if (row == SIZE / 2 && col == SIZE / 2) {
                line += "┼─";
            } else if (row == SIZE / 2) {
                line += "──";
            } else if (col == SIZE / 2) {
                line += "│ ";
            } else {
                line += "  ";
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
            hbox({text("   "), btn(vader5::PAD_Y, " Y ", Color::Yellow), text("   ")}),
            hbox({btn(vader5::PAD_X, " X ", Color::Blue), text("   "),
                  btn(vader5::PAD_B, " B ", Color::Red)}),
            hbox({text("   "), btn(vader5::PAD_A, " A ", Color::Green), text("   ")}),
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
            btn(vader5::PAD_LB, " LB "),
            render_trigger("LT", lt),
        }),
        text("     "),
        vbox({
            btn(vader5::PAD_RB, " RB "),
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
        btn(vader5::PAD_SELECT, " SELECT "),
        text("  "),
        btn(vader5::PAD_MODE, " MODE "),
        text("  "),
        btn(vader5::PAD_START, " START "),
    });
}

void input_thread(const vader5::Hidraw& hidraw) {
    try {
        std::array<uint8_t, READ_BUFFER_SIZE> buf{};
        while (g_running.load()) {
            auto bytes = hidraw.read(buf);
            if (!g_test_mode.load() && bytes && *bytes > 0) {
                if (auto state = vader5::Hidraw::parse_report({buf.data(), *bytes})) {
                    const std::scoped_lock lock(g_mutex);
                    g_state = *state;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    } catch (...) {
        g_running.store(false);
    }
}

constexpr uint8_t EXT_MAGIC_EF = 0xef;
constexpr size_t EXT_OFF_LX = 3;
constexpr size_t EXT_OFF_BTNS = 11;
constexpr size_t EXT_OFF_EXT1 = 13;
constexpr size_t EXT_OFF_EXT2 = 14;
constexpr size_t EXT_OFF_LT = 15;
constexpr size_t EXT_OFF_RT = 16;
constexpr size_t EXT_OFF_GYRO_X = 17;
constexpr size_t EXT_OFF_ACCEL_X = 23;
constexpr size_t EXT_REPORT_MIN = 29;

constexpr uint8_t EP2_B11_A = 0x10;
constexpr uint8_t EP2_B11_B = 0x20;
constexpr uint8_t EP2_B11_SELECT = 0x40;
constexpr uint8_t EP2_B11_X = 0x80;
constexpr uint8_t EP2_B12_Y = 0x01;
constexpr uint8_t EP2_B12_START = 0x02;
constexpr uint8_t EP2_B12_LB = 0x04;
constexpr uint8_t EP2_B12_RB = 0x08;
constexpr uint8_t EP2_B12_L3 = 0x40;
constexpr uint8_t EP2_B12_R3 = 0x80;

auto parse_ep2_buttons(uint8_t b11, uint8_t b12) -> uint16_t {
    uint16_t btns = 0;
    if ((b11 & EP2_B11_A) != 0) {
        btns |= vader5::PAD_A;
    }
    if ((b11 & EP2_B11_B) != 0) {
        btns |= vader5::PAD_B;
    }
    if ((b11 & EP2_B11_X) != 0) {
        btns |= vader5::PAD_X;
    }
    if ((b12 & EP2_B12_Y) != 0) {
        btns |= vader5::PAD_Y;
    }
    if ((b12 & EP2_B12_LB) != 0) {
        btns |= vader5::PAD_LB;
    }
    if ((b12 & EP2_B12_RB) != 0) {
        btns |= vader5::PAD_RB;
    }
    if ((b11 & EP2_B11_SELECT) != 0) {
        btns |= vader5::PAD_SELECT;
    }
    if ((b12 & EP2_B12_START) != 0) {
        btns |= vader5::PAD_START;
    }
    if ((b12 & EP2_B12_L3) != 0) {
        btns |= vader5::PAD_L3;
    }
    if ((b12 & EP2_B12_R3) != 0) {
        btns |= vader5::PAD_R3;
    }
    return btns;
}

auto parse_ep2_dpad(uint8_t b11) -> uint8_t {
    const uint8_t dpad_bits = b11 & 0x0F;
    constexpr std::array<uint8_t, 16> DPAD_MAP = {
        vader5::DPAD_NONE,       vader5::DPAD_UP,   vader5::DPAD_RIGHT,
        vader5::DPAD_UP_RIGHT,   vader5::DPAD_DOWN, vader5::DPAD_NONE,
        vader5::DPAD_DOWN_RIGHT, vader5::DPAD_NONE, vader5::DPAD_LEFT,
        vader5::DPAD_UP_LEFT,    vader5::DPAD_NONE, vader5::DPAD_NONE,
        vader5::DPAD_DOWN_LEFT,  vader5::DPAD_NONE, vader5::DPAD_NONE,
        vader5::DPAD_NONE};
    return DPAD_MAP.at(dpad_bits);
}

auto read_s16(std::span<const uint8_t, 2> data) -> int16_t {
    return static_cast<int16_t>(static_cast<uint16_t>(data.front()) |
                                (static_cast<uint16_t>(data.back()) << 8));
}

Element render_imu(const ImuData& imu, bool test_mode) {
    if (!test_mode) {
        return text("IMU: [T] to enable test mode") | dim;
    }
    constexpr int BAR_WIDTH = 10;
    constexpr int16_t RANGE = 4096;

    auto bar = [](int16_t val, int16_t range, Color neg_col, Color pos_col) {
        float ratio = static_cast<float>(val) / static_cast<float>(range);
        ratio = std::clamp(ratio, -1.0F, 1.0F);
        std::string left_str(BAR_WIDTH, ' ');
        std::string right_str(BAR_WIDTH, ' ');
        if (ratio < 0) {
            const int filled = static_cast<int>(-ratio * BAR_WIDTH);
            for (int i = BAR_WIDTH - filled; i < BAR_WIDTH; ++i) {
                left_str.at(static_cast<size_t>(i)) = '=';
            }
        } else {
            const int filled = static_cast<int>(ratio * BAR_WIDTH);
            for (int i = 0; i < filled; ++i) {
                right_str.at(static_cast<size_t>(i)) = '=';
            }
        }
        return hbox({
            text(left_str) | color(neg_col),
            text("|"),
            text(right_str) | color(pos_col),
        });
    };

    auto axis = [&](const std::string& name, int16_t val, Color neg, Color pos) {
        std::array<char, 16> buf{};
        (void)std::snprintf(buf.data(), buf.size(), "%+6d", val);
        return hbox(
            {text(name + ": "), bar(val, RANGE, neg, pos), text(" " + std::string(buf.data()))});
    };

    return vbox({
               text("Gyroscope") | bold,
               axis("X", imu.gyro_x, Color::Red, Color::Green),
               axis("Y", imu.gyro_y, Color::Red, Color::Green),
               axis("Z", imu.gyro_z, Color::Red, Color::Green),
               text("Accelerometer (4096=1g)") | bold,
               axis("X", imu.accel_x, Color::Blue, Color::Yellow),
               axis("Y", imu.accel_y, Color::Blue, Color::Yellow),
               axis("Z", imu.accel_z, Color::Blue, Color::Yellow),
           }) |
           border;
}

void parse_ext_report(std::span<const uint8_t, READ_BUFFER_SIZE> data) {
    vader5::GamepadState state{};
    state.left_x = read_s16(data.subspan<EXT_OFF_LX, 2>());
    state.left_y = static_cast<int16_t>(-read_s16(data.subspan<EXT_OFF_LX + 2, 2>()));
    state.right_x = read_s16(data.subspan<EXT_OFF_LX + 4, 2>());
    state.right_y = static_cast<int16_t>(-read_s16(data.subspan<EXT_OFF_LX + 6, 2>()));

    const uint8_t b11 = data[EXT_OFF_BTNS];
    const uint8_t b12 = data[EXT_OFF_BTNS + 1];
    state.buttons = parse_ep2_buttons(b11, b12);
    state.dpad = parse_ep2_dpad(b11);
    state.left_trigger = data[EXT_OFF_LT];
    state.right_trigger = data[EXT_OFF_RT];
    state.ext_buttons = data[EXT_OFF_EXT1];
    state.ext_buttons2 = data[EXT_OFF_EXT2];

    {
        const std::scoped_lock lock(g_mutex);
        g_state = state;
    }
    g_ext_buttons.store(state.ext_buttons);
    g_ext_buttons2.store(state.ext_buttons2);

    ImuData imu{};
    imu.gyro_x = read_s16(data.subspan<EXT_OFF_GYRO_X, 2>());
    imu.gyro_y = read_s16(data.subspan<EXT_OFF_GYRO_X + 2, 2>());
    imu.gyro_z = read_s16(data.subspan<EXT_OFF_GYRO_X + 4, 2>());
    imu.accel_x = read_s16(data.subspan<EXT_OFF_ACCEL_X, 2>());
    imu.accel_y = read_s16(data.subspan<EXT_OFF_ACCEL_X + 2, 2>());
    imu.accel_z = read_s16(data.subspan<EXT_OFF_ACCEL_X + 4, 2>());
    {
        const std::scoped_lock lock(g_imu_mutex);
        g_imu = imu;
    }
}

void config_thread(vader5::Hidraw& hidraw_cfg) {
    try {
        send_init_sequence(hidraw_cfg);
        std::array<uint8_t, READ_BUFFER_SIZE> buf{};
        while (g_running.load()) {
            if (g_test_mode_changed.exchange(false)) {
                send_test_mode(hidraw_cfg, g_test_mode.load());
            }
            if (g_test_mode.load()) {
                auto bytes = hidraw_cfg.read(buf);
                if (bytes && *bytes >= EXT_REPORT_MIN) {
                    if (buf[0] == MAGIC_5A && buf[1] == MAGIC_A5 && buf[2] == EXT_MAGIC_EF) {
                        parse_ext_report(std::span<const uint8_t, READ_BUFFER_SIZE>(buf));
                    }
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(READ_TIMEOUT_MS));
            }
        }
        send_test_mode(hidraw_cfg, false);
    } catch (...) {
        g_running.store(false);
    }
}

auto open_hidraw_input() -> std::optional<vader5::Hidraw> {
    auto hidraw = vader5::Hidraw::open(vader5::VENDOR_ID, vader5::PRODUCT_ID, IFACE_INPUT);
    if (!hidraw) {
        std::cerr << "Error: Failed to open hidraw (Interface 0)\n";
        return std::nullopt;
    }
    std::cerr << "Hidraw opened (Interface 0)\n";
    add_log("IF0: hidraw OK");
    return std::move(*hidraw);
}

auto open_hidraw_config() -> std::optional<vader5::Hidraw> {
    auto hidraw_cfg = vader5::Hidraw::open(vader5::VENDOR_ID, vader5::PRODUCT_ID, IFACE_CONFIG);
    if (!hidraw_cfg) {
        std::cerr << "Warning: Failed to open hidraw (Interface 1)\n";
        add_log("IF1: hidraw FAILED");
        return std::nullopt;
    }
    std::cerr << "Hidraw opened (Interface 1)\n";
    add_log("IF1: hidraw OK");
    return std::move(*hidraw_cfg);
}
} // namespace

auto main() -> int {
    auto hidraw_input = open_hidraw_input();
    if (!hidraw_input) {
        return EXIT_FAILURE;
    }

    auto hidraw_cfg = open_hidraw_config();

    std::optional<std::thread> cfg_handler;
    if (hidraw_cfg) {
        cfg_handler.emplace(config_thread, std::ref(*hidraw_cfg));
    }
    std::thread reader(input_thread, std::cref(*hidraw_input));

    auto screen = ScreenInteractive::Fullscreen();

    auto renderer = Renderer([&] {
        vader5::GamepadState st;
        {
            const std::scoped_lock lock(g_mutex);
            st = g_state;
        }

        const bool l3 = (st.buttons & vader5::PAD_L3) != 0;
        const bool r3 = (st.buttons & vader5::PAD_R3) != 0;
        const uint8_t ext1 = g_ext_buttons.load();
        const uint8_t ext2 = g_ext_buttons2.load();
        const bool test_mode = g_test_mode.load();
        const ImuData imu = [&] {
            const std::scoped_lock lock(g_imu_mutex);
            return g_imu;
        }();
        const std::vector<std::string> logs = [&] {
            const std::scoped_lock lock(g_log_mutex);
            return g_logs;
        }();

        std::string title = "═══ Vader 5 Pro Debug";
        if (test_mode) {
            title += " [TEST MODE]";
        }
        title += " ═══";

        return vbox({
                   text(title) | bold | center,
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
                   render_ext_buttons(ext1, ext2, test_mode) | center,
                   separator(),
                   render_imu(imu, test_mode) | center,
                   text("[T] test mode  [Q] quit") | dim | center,
                   separator(),
                   vbox([&] {
                       std::vector<Element> log_elems;
                       log_elems.reserve(logs.size() + 1);
                       for (const auto& msg : logs) {
                           log_elems.push_back(text(msg) | dim);
                       }
                       if (log_elems.empty()) {
                           log_elems.push_back(text("Ready") | dim);
                       }
                       return log_elems;
                   }()) |
                       center,
               }) |
               border | center;
    });

    auto component = CatchEvent(renderer, [&](const Event& event) {
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            g_running.store(false);
            screen.Exit();
            return true;
        }
        if (event.character().size() == 1) {
            const char ch = event.character().front();
            if (ch == 't' || ch == 'T') {
                bool expected = g_test_mode.load();
                while (!g_test_mode.compare_exchange_weak(expected, !expected)) {
                }
                g_test_mode_changed.store(true);
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
    if (cfg_handler) {
        cfg_handler->join();
    }
    return 0;
}
