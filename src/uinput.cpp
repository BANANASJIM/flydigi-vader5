#include "vader5/uinput.hpp"

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>

namespace vader5 {

namespace {
constexpr int AXIS_MIN = -32768;
constexpr int AXIS_MAX = 32767;
constexpr int AXIS_FUZZ = 16;
constexpr int AXIS_FLAT = 128;
constexpr int TRIGGER_MIN = 0;
constexpr int TRIGGER_MAX = 255;
constexpr int EXT_BUTTON_COUNT = 8;
constexpr std::array<uint8_t, 8> EXT_MASKS = {EXT_C,  EXT_Z,      EXT_M4,   EXT_M1,
                                               EXT_M2, EXT_M3, EXT_CIRCLE, EXT_HOME};
} // namespace

auto Uinput::create(std::span<const std::optional<int>> ext_mappings, const char* name)
    -> Result<Uinput> {
    const int file_descriptor = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (file_descriptor < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    (void)ioctl(file_descriptor, UI_SET_EVBIT, EV_KEY);
    (void)ioctl(file_descriptor, UI_SET_EVBIT, EV_ABS);
    (void)ioctl(file_descriptor, UI_SET_EVBIT, EV_SYN);

    for (const int btn : {BTN_SOUTH, BTN_EAST, BTN_NORTH, BTN_WEST, BTN_TL, BTN_TR, BTN_SELECT,
                          BTN_START, BTN_MODE, BTN_THUMBL, BTN_THUMBR}) {
        (void)ioctl(file_descriptor, UI_SET_KEYBIT, btn);
    }

    for (int idx = 0; idx < EXT_BUTTON_COUNT; ++idx) {
        (void)ioctl(file_descriptor, UI_SET_KEYBIT, BTN_TRIGGER_HAPPY1 + idx);
    }

    for (const auto& mapping : ext_mappings) {
        if (mapping.has_value()) {
            (void)ioctl(file_descriptor, UI_SET_KEYBIT, *mapping);
        }
    }

    (void)ioctl(file_descriptor, UI_SET_KEYBIT, BTN_DPAD_UP);
    (void)ioctl(file_descriptor, UI_SET_KEYBIT, BTN_DPAD_DOWN);
    (void)ioctl(file_descriptor, UI_SET_KEYBIT, BTN_DPAD_LEFT);
    (void)ioctl(file_descriptor, UI_SET_KEYBIT, BTN_DPAD_RIGHT);

    uinput_abs_setup abs_setup{};

    auto setup_axis = [&](int code, int min_val, int max_val) {
        abs_setup.code = static_cast<uint16_t>(code);
        abs_setup.absinfo.minimum = min_val;
        abs_setup.absinfo.maximum = max_val;
        abs_setup.absinfo.fuzz = AXIS_FUZZ;
        abs_setup.absinfo.flat = AXIS_FLAT;
        (void)ioctl(file_descriptor, UI_ABS_SETUP, &abs_setup);
    };

    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_X);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_Y);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_RX);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_RY);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_Z);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_RZ);

    setup_axis(ABS_X, AXIS_MIN, AXIS_MAX);
    setup_axis(ABS_Y, AXIS_MIN, AXIS_MAX);
    setup_axis(ABS_RX, AXIS_MIN, AXIS_MAX);
    setup_axis(ABS_RY, AXIS_MIN, AXIS_MAX);
    setup_axis(ABS_Z, TRIGGER_MIN, TRIGGER_MAX);
    setup_axis(ABS_RZ, TRIGGER_MIN, TRIGGER_MAX);

    uinput_setup setup{};
    std::strncpy(setup.name, name, UINPUT_MAX_NAME_SIZE - 1);
    setup.id.bustype = BUS_USB;
    setup.id.vendor = VENDOR_ID;
    setup.id.product = PRODUCT_ID;
    setup.id.version = 1;

    (void)ioctl(file_descriptor, UI_DEV_SETUP, &setup);
    (void)ioctl(file_descriptor, UI_DEV_CREATE);

    return Uinput(file_descriptor, ext_mappings);
}

Uinput::Uinput(int file_descriptor, std::span<const std::optional<int>> mappings)
    : fd_(file_descriptor) {
    std::ranges::copy(mappings, ext_mappings_.begin());
}

Uinput::~Uinput() {
    if (fd_ >= 0) {
        (void)ioctl(fd_, UI_DEV_DESTROY);
        ::close(fd_);
    }
}

Uinput::Uinput(Uinput&& other) noexcept : fd_(other.fd_), ext_mappings_(other.ext_mappings_) {
    other.fd_ = -1;
}

auto Uinput::operator=(Uinput&& other) noexcept -> Uinput& {
    if (this != &other) {
        if (fd_ >= 0) {
            (void)ioctl(fd_, UI_DEV_DESTROY);
            ::close(fd_);
        }
        fd_ = other.fd_;
        ext_mappings_ = other.ext_mappings_;
        other.fd_ = -1;
    }
    return *this;
}

void Uinput::emit_key(int code, int value) const {
    input_event event{};
    event.type = EV_KEY;
    event.code = static_cast<uint16_t>(code);
    event.value = value;
    (void)::write(fd_, &event, sizeof(event));
}

void Uinput::emit_abs(int code, int value) const {
    input_event event{};
    event.type = EV_ABS;
    event.code = static_cast<uint16_t>(code);
    event.value = value;
    (void)::write(fd_, &event, sizeof(event));
}

void Uinput::sync() const {
    input_event event{};
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    (void)::write(fd_, &event, sizeof(event));
}

namespace {
auto is_dpad_up(uint8_t dpad) -> bool {
    return dpad == DPAD_UP || dpad == DPAD_UP_LEFT || dpad == DPAD_UP_RIGHT;
}
auto is_dpad_down(uint8_t dpad) -> bool {
    return dpad == DPAD_DOWN || dpad == DPAD_DOWN_LEFT || dpad == DPAD_DOWN_RIGHT;
}
auto is_dpad_left(uint8_t dpad) -> bool {
    return dpad == DPAD_LEFT || dpad == DPAD_UP_LEFT || dpad == DPAD_DOWN_LEFT;
}
auto is_dpad_right(uint8_t dpad) -> bool {
    return dpad == DPAD_RIGHT || dpad == DPAD_UP_RIGHT || dpad == DPAD_DOWN_RIGHT;
}
} // namespace

auto Uinput::emit(const GamepadState& state, const GamepadState& prev) const -> Result<void> {
    if (state.left_x != prev.left_x) {
        emit_abs(ABS_X, state.left_x);
    }
    if (state.left_y != prev.left_y) {
        emit_abs(ABS_Y, state.left_y);
    }
    if (state.right_x != prev.right_x) {
        emit_abs(ABS_RX, state.right_x);
    }
    if (state.right_y != prev.right_y) {
        emit_abs(ABS_RY, state.right_y);
    }
    if (state.left_trigger != prev.left_trigger) {
        emit_abs(ABS_Z, state.left_trigger);
    }
    if (state.right_trigger != prev.right_trigger) {
        emit_abs(ABS_RZ, state.right_trigger);
    }

    auto emit_btn = [&](uint16_t mask, int code) {
        const bool curr = (state.buttons & mask) != 0;
        const bool old = (prev.buttons & mask) != 0;
        if (curr != old) {
            emit_key(code, curr ? 1 : 0);
        }
    };

    emit_btn(BTN_A, BTN_SOUTH);
    emit_btn(BTN_B, BTN_EAST);
    emit_btn(BTN_X, BTN_WEST);
    emit_btn(BTN_Y, BTN_NORTH);
    emit_btn(BTN_LB, BTN_TL);
    emit_btn(BTN_RB, BTN_TR);
    emit_btn(BTN_SELECT, BTN_SELECT);
    emit_btn(BTN_START, BTN_START);
    emit_btn(BTN_MODE, BTN_MODE);
    emit_btn(BTN_L3, BTN_THUMBL);
    emit_btn(BTN_R3, BTN_THUMBR);

    for (size_t idx = 0; idx < EXT_BUTTON_COUNT; ++idx) {
        const bool curr = (state.ext_buttons & EXT_MASKS[idx]) != 0;
        const bool old = (prev.ext_buttons & EXT_MASKS[idx]) != 0;
        if (curr != old) {
            const int code = ext_mappings_[idx].value_or(BTN_TRIGGER_HAPPY1 + static_cast<int>(idx));
            emit_key(code, curr ? 1 : 0);
        }
    }

    if (state.dpad != prev.dpad) {
        const bool dir_up = is_dpad_up(state.dpad);
        const bool dir_down = is_dpad_down(state.dpad);
        const bool dir_left = is_dpad_left(state.dpad);
        const bool dir_right = is_dpad_right(state.dpad);
        const bool old_up = is_dpad_up(prev.dpad);
        const bool old_down = is_dpad_down(prev.dpad);
        const bool old_left = is_dpad_left(prev.dpad);
        const bool old_right = is_dpad_right(prev.dpad);

        if (dir_up != old_up) {
            emit_key(BTN_DPAD_UP, dir_up ? 1 : 0);
        }
        if (dir_down != old_down) {
            emit_key(BTN_DPAD_DOWN, dir_down ? 1 : 0);
        }
        if (dir_left != old_left) {
            emit_key(BTN_DPAD_LEFT, dir_left ? 1 : 0);
        }
        if (dir_right != old_right) {
            emit_key(BTN_DPAD_RIGHT, dir_right ? 1 : 0);
        }
    }

    sync();
    return {};
}

} // namespace vader5
