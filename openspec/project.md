# Project Context

## Purpose
Linux driver and tools for Flydigi Vader 5 Pro gamepad (2.4G USB).

## Device Info
- VID: 0x37d7, PID: 0x2401
- Interface 0: Vendor Specific (Xbox 0x5d) - Main input (20B on EP1 IN)
- Interface 1-3: HID - Config/extended/keyboard emulation

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Vader 5 Pro USB                         │
├──────────────────┬──────────────────┬───────────────────────┤
│   Interface 0    │   Interface 1    │    Interface 2-3      │
│ Vendor (Xbox)    │      HID         │        HID            │
│   EP1 IN 20B     │   EP2 IN/OUT     │    EP3/4 IN           │
└────────┬─────────┴────────┬─────────┴───────────────────────┘
         │                  │
         ▼                  ▼
┌─────────────────┐  ┌─────────────────┐
│  Kernel Driver  │  │ Userspace Tools │
│  (driver/)      │  │ (src/, hidraw)  │
│                 │  │                 │
│ USB driver      │  │ vader5-debug    │
│ → input event   │  │ vader5-config   │
└─────────────────┘  └─────────────────┘
```

## Components

### Kernel Driver (`driver/`)
- USB driver (not HID) for Interface 0
- Xbox-like vendor protocol
- Creates `/dev/input/eventX`

### Userspace Tools (`src/`)
- `vader5-debug` - Protocol analysis via hidraw
- `vader5-config` - LED/vibration/key mapping

## Tech Stack
| Component | Language | API |
|-----------|----------|-----|
| Kernel driver | C | USB, input subsystem |
| Userspace tools | C++20 | hidraw, uinput |

## Code Style
- Kernel: Linux kernel style (tabs, 80 cols)
- Userspace: clang-format, C++20

## References
- vader3: https://github.com/ahungry/vader3
- flydigictl: https://github.com/pipe01/flydigictl
- xpadneo: https://github.com/atar-axis/xpadneo
