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

inline void write_event(int fd, const input_event& ev) {
    if (::write(fd, &ev, sizeof(ev)) < 0) {}
}
constexpr int AXIS_MIN = -32768;
constexpr int AXIS_MAX = 32767;
constexpr int AXIS_FUZZ = 16;
constexpr int AXIS_FLAT = 128;
constexpr int TRIGGER_MIN = 0;
constexpr int TRIGGER_MAX = 255;
constexpr int EXT_BUTTON_COUNT = 8;
constexpr std::array<uint8_t, 8> EXT_MASKS = {EXT_C,  EXT_Z,  EXT_M1, EXT_M2,
                                              EXT_M3, EXT_M4, EXT_LM, EXT_RM};
// Default codes: M1-M4 use BTN_TRIGGER_HAPPY5-8 to match xpad Elite paddles
// Note: Hardware bit3=M3, bit4=M2 (reversed from label)
constexpr std::array<int, 8> DEFAULT_EXT_CODES = {
    BTN_TRIGGER_HAPPY1, BTN_TRIGGER_HAPPY2, // C, Z
    BTN_TRIGGER_HAPPY5, BTN_TRIGGER_HAPPY7, // M1, M3 (Elite P1, P3)
    BTN_TRIGGER_HAPPY6, BTN_TRIGGER_HAPPY8, // M2, M4 (Elite P2, P4)
    BTN_TRIGGER_HAPPY3, BTN_TRIGGER_HAPPY4, // LM, RM
};
} // namespace

auto Uinput::create(std::span<const std::optional<int>> ext_mappings, const char* name)
    -> Result<Uinput> {
    const int file_descriptor = ::open("/dev/uinput", O_RDWR | O_NONBLOCK);
    if (file_descriptor < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    (void)ioctl(file_descriptor, UI_SET_EVBIT, EV_KEY);
    (void)ioctl(file_descriptor, UI_SET_EVBIT, EV_ABS);
    (void)ioctl(file_descriptor, UI_SET_EVBIT, EV_SYN);
    (void)ioctl(file_descriptor, UI_SET_EVBIT, EV_FF);
    (void)ioctl(file_descriptor, UI_SET_FFBIT, FF_RUMBLE);

    for (const int btn : {BTN_SOUTH, BTN_EAST, BTN_NORTH, BTN_WEST, BTN_TL, BTN_TR, BTN_SELECT,
                          BTN_START, BTN_MODE, BTN_THUMBL, BTN_THUMBR}) {
        (void)ioctl(file_descriptor, UI_SET_KEYBIT, btn);
    }

    for (int idx = 0; idx < 10; ++idx) {
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

    auto setup_stick = [&](int code) {
        abs_setup.code = static_cast<uint16_t>(code);
        abs_setup.absinfo.minimum = AXIS_MIN;
        abs_setup.absinfo.maximum = AXIS_MAX;
        abs_setup.absinfo.fuzz = AXIS_FUZZ;
        abs_setup.absinfo.flat = AXIS_FLAT;
        (void)ioctl(file_descriptor, UI_ABS_SETUP, &abs_setup);
    };

    auto setup_trigger = [&](int code) {
        abs_setup.code = static_cast<uint16_t>(code);
        abs_setup.absinfo.minimum = TRIGGER_MIN;
        abs_setup.absinfo.maximum = TRIGGER_MAX;
        abs_setup.absinfo.fuzz = 0;
        abs_setup.absinfo.flat = 0;
        (void)ioctl(file_descriptor, UI_ABS_SETUP, &abs_setup);
    };

    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_X);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_Y);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_RX);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_RY);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_Z);
    (void)ioctl(file_descriptor, UI_SET_ABSBIT, ABS_RZ);

    setup_stick(ABS_X);
    setup_stick(ABS_Y);
    setup_stick(ABS_RX);
    setup_stick(ABS_RY);
    setup_trigger(ABS_Z);
    setup_trigger(ABS_RZ);

    uinput_setup setup{};
    std::strncpy(setup.name, name, UINPUT_MAX_NAME_SIZE - 1);
    setup.name[UINPUT_MAX_NAME_SIZE - 1] = '\0';
    setup.id.bustype = BUS_USB;
    setup.id.vendor = ELITE_VENDOR_ID;
    setup.id.product = ELITE_PRODUCT_ID;
    setup.id.version = 1;
    setup.ff_effects_max = 16;

    (void)ioctl(file_descriptor, UI_DEV_SETUP, &setup);
    if (ioctl(file_descriptor, UI_DEV_CREATE) < 0) {
        ::close(file_descriptor);
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    return Uinput(file_descriptor, ext_mappings);
}

Uinput::Uinput(int file_descriptor, std::span<const std::optional<int>> mappings)
    : fd_(file_descriptor) {
    auto count = std::min(mappings.size(), ext_mappings_.size());
    std::ranges::copy_n(mappings.begin(), static_cast<std::ptrdiff_t>(count),
                        ext_mappings_.begin());
}

Uinput::~Uinput() {
    if (fd_ >= 0) {
        (void)ioctl(fd_, UI_DEV_DESTROY);
        ::close(fd_);
    }
}

Uinput::Uinput(Uinput&& other) noexcept
    : fd_(other.fd_), ext_mappings_(other.ext_mappings_), ff_effects_(other.ff_effects_) {
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
        ff_effects_ = other.ff_effects_;
        other.fd_ = -1;
    }
    return *this;
}

void Uinput::emit_key(int code, int value) const {
    input_event event{};
    event.type = EV_KEY;
    event.code = static_cast<uint16_t>(code);
    event.value = value;
    write_event(fd_, event);
}

void Uinput::emit_abs(int code, int value) const {
    input_event event{};
    event.type = EV_ABS;
    event.code = static_cast<uint16_t>(code);
    event.value = value;
    write_event(fd_, event);
}

void Uinput::sync() const {
    input_event event{};
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    write_event(fd_, event);
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

    emit_btn(PAD_A, BTN_SOUTH);
    emit_btn(PAD_B, BTN_EAST);
    emit_btn(PAD_X, BTN_NORTH);
    emit_btn(PAD_Y, BTN_WEST);
    emit_btn(PAD_LB, BTN_TL);
    emit_btn(PAD_RB, BTN_TR);
    emit_btn(PAD_SELECT, BTN_SELECT);
    emit_btn(PAD_START, BTN_START);
    emit_btn(PAD_L3, BTN_THUMBL);
    emit_btn(PAD_R3, BTN_THUMBR);

    for (size_t idx = 0; idx < EXT_BUTTON_COUNT; ++idx) {
        const bool curr = (state.ext_buttons & EXT_MASKS[idx]) != 0;
        const bool old = (prev.ext_buttons & EXT_MASKS[idx]) != 0;
        if (curr != old) {
            const int code = ext_mappings_[idx].value_or(DEFAULT_EXT_CODES[idx]);
            emit_key(code, curr ? 1 : 0);
        }
    }

    if (state.ext_buttons2 != prev.ext_buttons2) {
        const bool curr_o = (state.ext_buttons2 & EXT_O) != 0;
        const bool old_o = (prev.ext_buttons2 & EXT_O) != 0;
        if (curr_o != old_o) {
            emit_key(BTN_TRIGGER_HAPPY9, curr_o ? 1 : 0);
        }

        const bool curr_home = (state.ext_buttons2 & EXT_HOME) != 0;
        const bool old_home = (prev.ext_buttons2 & EXT_HOME) != 0;
        if (curr_home != old_home) {
            emit_key(BTN_MODE, curr_home ? 1 : 0);
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

auto Uinput::poll_ff() -> std::optional<RumbleEffect> {
    std::optional<RumbleEffect> result;
    input_event ev{};

    while (true) {
        const auto bytes = ::read(fd_, &ev, sizeof(ev));
        if (bytes != sizeof(ev)) {
            break;
        }

        if (ev.type == EV_UINPUT && ev.code == UI_FF_UPLOAD) {
            uinput_ff_upload upload{};
            upload.request_id = static_cast<__u32>(ev.value);
            if (ioctl(fd_, UI_BEGIN_FF_UPLOAD, &upload) < 0) {
                continue;
            }
            if (upload.effect.type == FF_RUMBLE && upload.effect.id >= 0 &&
                static_cast<size_t>(upload.effect.id) < ff_effects_.size()) {
                ff_effects_[static_cast<size_t>(upload.effect.id)] = {
                    upload.effect.u.rumble.strong_magnitude,
                    upload.effect.u.rumble.weak_magnitude,
                };
            }
            upload.retval = 0;
            (void)ioctl(fd_, UI_END_FF_UPLOAD, &upload);
            continue;
        }

        if (ev.type == EV_UINPUT && ev.code == UI_FF_ERASE) {
            uinput_ff_erase erase{};
            erase.request_id = static_cast<__u32>(ev.value);
            if (ioctl(fd_, UI_BEGIN_FF_ERASE, &erase) < 0) {
                continue;
            }
            if (erase.effect_id < ff_effects_.size()) {
                ff_effects_[erase.effect_id] = {};
            }
            erase.retval = 0;
            (void)ioctl(fd_, UI_END_FF_ERASE, &erase);
            continue;
        }

        if (ev.type == EV_FF && ev.code < ff_effects_.size()) {
            result = ev.value > 0 ? ff_effects_[ev.code] : RumbleEffect{0, 0};
        }
    }

    return result;
}

// InputDevice - separate mouse/keyboard device
auto InputDevice::create(const char* name) -> Result<InputDevice> {
    const int fd = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    (void)ioctl(fd, UI_SET_EVBIT, EV_KEY);
    (void)ioctl(fd, UI_SET_EVBIT, EV_REL);
    (void)ioctl(fd, UI_SET_EVBIT, EV_SYN);

    // Mouse
    (void)ioctl(fd, UI_SET_RELBIT, REL_X);
    (void)ioctl(fd, UI_SET_RELBIT, REL_Y);
    (void)ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
    (void)ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);
    for (const int btn : {BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA, BTN_FORWARD, BTN_BACK}) {
        (void)ioctl(fd, UI_SET_KEYBIT, btn);
    }

    // Keyboard - register common keys
    for (int key = KEY_ESC; key <= KEY_KPDOT; ++key) {
        (void)ioctl(fd, UI_SET_KEYBIT, key);
    }
    for (int key = KEY_F13; key <= KEY_F24; ++key) {
        (void)ioctl(fd, UI_SET_KEYBIT, key);
    }
    for (int key : {KEY_RIGHTCTRL, KEY_RIGHTALT, KEY_LEFTMETA, KEY_RIGHTMETA,
                    KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_LEFT, KEY_RIGHT,
                    KEY_END, KEY_DOWN, KEY_PAGEDOWN, KEY_INSERT, KEY_DELETE}) {
        (void)ioctl(fd, UI_SET_KEYBIT, key);
    }

    uinput_setup setup{};
    std::strncpy(setup.name, name, UINPUT_MAX_NAME_SIZE - 1);
    setup.name[UINPUT_MAX_NAME_SIZE - 1] = '\0';
    setup.id.bustype = BUS_USB;
    setup.id.vendor = 0x0001;
    setup.id.product = 0x0001;
    setup.id.version = 1;

    (void)ioctl(fd, UI_DEV_SETUP, &setup);
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        ::close(fd);
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    return InputDevice(fd);
}

InputDevice::~InputDevice() {
    if (fd_ >= 0) {
        (void)ioctl(fd_, UI_DEV_DESTROY);
        ::close(fd_);
    }
}

InputDevice::InputDevice(InputDevice&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

auto InputDevice::operator=(InputDevice&& other) noexcept -> InputDevice& {
    if (this != &other) {
        if (fd_ >= 0) {
            (void)ioctl(fd_, UI_DEV_DESTROY);
            ::close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void InputDevice::emit_rel(int code, int value) const {
    if (value == 0) {
        return;
    }
    input_event event{};
    event.type = EV_REL;
    event.code = static_cast<uint16_t>(code);
    event.value = value;
    write_event(fd_, event);
}

void InputDevice::emit_key(int code, int value) const {
    input_event event{};
    event.type = EV_KEY;
    event.code = static_cast<uint16_t>(code);
    event.value = value;
    write_event(fd_, event);
}

void InputDevice::move_mouse(int dx, int dy) const {
    emit_rel(REL_X, dx);
    emit_rel(REL_Y, dy);
}

void InputDevice::scroll(int vertical, int horizontal) const {
    emit_rel(REL_WHEEL, vertical);
    emit_rel(REL_HWHEEL, horizontal);
}

void InputDevice::click(int code, bool pressed) const {
    emit_key(code, pressed ? 1 : 0);
}

void InputDevice::key(int code, bool pressed) const {
    emit_key(code, pressed ? 1 : 0);
}

void InputDevice::sync() const {
    input_event event{};
    event.type = EV_SYN;
    event.code = SYN_REPORT;
    write_event(fd_, event);
}

} // namespace vader5
