#include "vader5/mouse.hpp"

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>

namespace vader5 {

auto Mouse::create(const char* name) -> Result<Mouse> {
    const int fd = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        return std::unexpected(std::error_code(errno, std::system_category()));
    }

    (void)ioctl(fd, UI_SET_EVBIT, EV_KEY);
    (void)ioctl(fd, UI_SET_EVBIT, EV_REL);
    (void)ioctl(fd, UI_SET_EVBIT, EV_SYN);

    (void)ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    (void)ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    (void)ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);

    (void)ioctl(fd, UI_SET_RELBIT, REL_X);
    (void)ioctl(fd, UI_SET_RELBIT, REL_Y);
    (void)ioctl(fd, UI_SET_RELBIT, REL_WHEEL);

    uinput_setup setup{};
    std::strncpy(setup.name, name, UINPUT_MAX_NAME_SIZE - 1);
    setup.id.bustype = BUS_USB;
    setup.id.vendor = 0x37d7;
    setup.id.product = 0x2402;
    setup.id.version = 1;

    (void)ioctl(fd, UI_DEV_SETUP, &setup);
    (void)ioctl(fd, UI_DEV_CREATE);

    return Mouse(fd);
}

Mouse::~Mouse() {
    if (fd_ >= 0) {
        (void)ioctl(fd_, UI_DEV_DESTROY);
        ::close(fd_);
    }
}

Mouse::Mouse(Mouse&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

auto Mouse::operator=(Mouse&& other) noexcept -> Mouse& {
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

void Mouse::move(int dx, int dy) const {
    input_event ev{};
    if (dx != 0) {
        ev.type = EV_REL;
        ev.code = REL_X;
        ev.value = dx;
        (void)::write(fd_, &ev, sizeof(ev));
    }
    if (dy != 0) {
        ev.type = EV_REL;
        ev.code = REL_Y;
        ev.value = dy;
        (void)::write(fd_, &ev, sizeof(ev));
    }
}

void Mouse::button(int code, bool pressed) const {
    input_event ev{};
    ev.type = EV_KEY;
    ev.code = static_cast<uint16_t>(code);
    ev.value = pressed ? 1 : 0;
    (void)::write(fd_, &ev, sizeof(ev));
}

void Mouse::scroll(int delta) const {
    if (delta == 0)
        return;
    input_event ev{};
    ev.type = EV_REL;
    ev.code = REL_WHEEL;
    ev.value = delta;
    (void)::write(fd_, &ev, sizeof(ev));
}

void Mouse::sync() const {
    input_event ev{};
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    (void)::write(fd_, &ev, sizeof(ev));
}

} // namespace vader5