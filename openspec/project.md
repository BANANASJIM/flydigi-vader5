# Project Context

## Purpose
Linux userspace driver for Flydigi Vader 5 Pro gamepad (2.4G USB).

## Device Info
- VID: 0x37d7, PID: 0x2401
- Interface 0: Vendor (Xbox 0x5d) - EP1 IN 20B 主输入, EP5 OUT 8B 震动 (libusb)
- Interface 1: HID - EP2 IN 32B 配置响应/扩展输入, EP6 OUT 32B 配置命令 (hidraw)
- Interface 2: HID - EP2 IN 32B 扩展输入 (hidraw, 实际数据从 IF1 返回)

## Architecture

```
┌──────────────────────────────────────────────────────────────────┐
│                       Vader 5 Pro USB                            │
├──────────────────┬──────────────────┬────────────────────────────┤
│   Interface 0    │   Interface 1    │       Interface 2          │
│ Vendor (libusb)  │  HID (hidraw)    │      HID (hidraw)          │
│ EP1 IN: 主输入   │ EP6 OUT: 命令    │    EP2 IN: (未使用)        │
│ EP5 OUT: 震动    │ EP2 IN: 响应/扩展│                            │
└────────┬─────────┴────────┬─────────┴────────────────────────────┘
         │                  │
         ▼                  ▼
┌─────────────────┐  ┌─────────────────┐
│   Transport     │  │     Hidraw      │
│   (libusb)      │  │ 命令发送/扩展读取│
└────────┬────────┘  └────────┬────────┘
         │                    │
         │             GamepadState
         │                    │
         ▼                    ▼
┌─────────────────┐   ┌─────────────────┐
│  vader5-debug   │   │     Uinput      │
│   TUI Tool      │   │ Virtual Gamepad │
└─────────────────┘   └────────┬────────┘
                               │
                               ▼
                      ┌─────────────────┐
                      │    vader5d      │
                      │  Main Daemon    │
                      └─────────────────┘
```

## Components

### Core Library (`include/vader5/`, `src/`)
| Module | File | Purpose |
|--------|------|---------|
| Types | `types.hpp` | GamepadState, Button/Dpad enums, Result<T> |
| Hidraw | `hidraw.hpp/cpp` | HID device I/O, report parsing |
| Uinput | `uinput.hpp/cpp` | Virtual gamepad, differential event emission |
| Gamepad | `gamepad.hpp/cpp` | Orchestrates hidraw → uinput pipeline |
| Config | `config.hpp/cpp` | TOML config, key remapping |
| Transport | `transport.hpp/cpp` | libusb wrapper for direct USB access |

### Executables
| Binary | Source | Purpose |
|--------|--------|---------|
| `vader5d` | `main.cpp` | Daemon: poll-loop forwarding to uinput |
| `vader5-debug` | `debug.cpp` | TUI: live state visualization, rumble control |

## Tech Stack
| Component | Technology | Purpose |
|-----------|------------|---------|
| Build | CMake 3.20+ | C++23 compilation |
| Config | toml++ v3.4.0 | TOML parsing |
| Debug UI | FTXUI v5.0.0 | Terminal UI |
| USB | libusb 1.0 | Direct device access (debug) |
| Input | Linux hidraw/uinput | Device abstraction |

## Code Style
- clang-format, C++23
- Headers: declarations only, minimal comments
- `Result<T>` = `std::expected<T, std::error_code>`

## References
- vader3: https://github.com/ahungry/vader3
- flydigictl: https://github.com/pipe01/flydigictl
- xpadneo: https://github.com/atar-axis/xpadneo
