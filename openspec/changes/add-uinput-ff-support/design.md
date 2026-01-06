# Design: Uinput Force Feedback

## Current State

```
Game → FF event → /dev/input/eventX → (dropped, uinput write-only)
```

## Target State

```
Game → FF event → /dev/input/eventX → uinput fd (read) → poll_ff() → send_rumble() → Hardware
```

## Implementation

### 1. Uinput Changes

**Open mode**: `O_WRONLY` → `O_RDWR`

**FF capability registration**:
```cpp
ioctl(fd, UI_SET_EVBIT, EV_FF);
ioctl(fd, UI_SET_FFBIT, FF_RUMBLE);
setup.ff_effects_max = 16;
```

**FF event handling** (`poll_ff()`):
- Read `input_event` from uinput fd
- Handle `EV_UINPUT` + `UI_FF_UPLOAD`: store effect in `ff_effects_[]`, reply with `uinput_ff_upload`
- Handle `EV_FF`: play/stop effect, return `RumbleEffect` for active rumble

### 2. Main Loop Changes

**Poll two fds**:
```cpp
pollfd pfds[2] = {
    {.fd = gamepad->fd(), .events = POLLIN},      // hidraw
    {.fd = gamepad->ff_fd(), .events = POLLIN},   // uinput
};
poll(pfds, 2, timeout);
```

**Process FF events**:
```cpp
if (pfds[1].revents & POLLIN) {
    if (auto rumble = gamepad->poll_ff()) {
        gamepad->send_rumble(rumble->strong >> 8, rumble->weak >> 8);
    }
}
```

## Data Flow

```
┌─────────┐   FF_RUMBLE    ┌──────────┐   UI_FF_UPLOAD   ┌──────────┐
│  Game   │ ─────────────► │  Kernel  │ ───────────────► │  uinput  │
│ (SDL)   │                │  evdev   │                  │   fd     │
└─────────┘                └──────────┘                  └────┬─────┘
                                                              │
                                                         poll_ff()
                                                              │
                                                              ▼
                                                       ┌──────────┐
                                                       │ send_    │
                                                       │ rumble() │
                                                       └────┬─────┘
                                                            │
                                                     5a a5 12 06 LL RR
                                                            │
                                                            ▼
                                                       ┌──────────┐
                                                       │ Hardware │
                                                       └──────────┘
```
