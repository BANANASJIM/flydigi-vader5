# Refactor Architecture

## Goals
- Support Steam Input (via uinput virtual gamepad)
- Minimal dependencies: libusb-1.0, linux headers only
- 2.4G USB protocol support (current)
- Reserved interfaces for Bluetooth (future)
- Modern C++23, strict clang-tidy

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                      Application                         │
│                       (main.cpp)                         │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────┐
│                        Device                            │
│         Combines Transport + Protocol + Output           │
└─────────────────────────────────────────────────────────┘
           │                │                │
           ▼                ▼                ▼
┌─────────────────┐ ┌──────────────┐ ┌──────────────────┐
│    Transport    │ │   Protocol   │ │      Output      │
│   (抽象接口)     │ │  (解析协议)   │ │   (uinput)       │
└─────────────────┘ └──────────────┘ └──────────────────┘
         ▲                 ▲
         │                 │
    ┌────┴────┐      ┌─────┴─────┐
    │  Usb    │      │  Parser   │
    │(libusb) │      │  (24g/bt) │
    └─────────┘      └───────────┘
    │  Bluetooth │ (future)
    └────────────┘
```

## Core Types

```cpp
// Result type for error handling
template <typename T>
using Result = std::expected<T, std::error_code>;

// Gamepad state
struct GamepadState {
    int16_t left_x, left_y, right_x, right_y;
    uint8_t left_trigger, right_trigger;
    uint16_t buttons;
    uint8_t dpad;
    uint8_t ext_buttons;  // M1-M4, C, Z, Circle, Home
};
```

## Transport Layer

```cpp
// Abstract interface using concepts
template <typename T>
concept TransportLike = requires(T t, std::span<uint8_t> buf) {
    { t.read(buf) } -> std::same_as<Result<size_t>>;
    { t.is_open() } -> std::same_as<bool>;
};

// USB implementation (libusb)
class UsbTransport {
public:
    static auto open(uint16_t vid, uint16_t pid, uint8_t iface, uint8_t endpoint)
        -> Result<UsbTransport>;
    auto read(std::span<uint8_t> buf) -> Result<size_t>;
    [[nodiscard]] auto is_open() const -> bool;
};

// Future: Bluetooth implementation
// class BluetoothTransport { ... };
```

## Protocol Layer

```cpp
namespace protocol {
    // Protocol identifier
    enum class Type { usb_24g, bluetooth };

    // Parse raw data to GamepadState
    auto parse(Type type, std::span<const uint8_t> data) -> std::optional<GamepadState>;
}
```

## Output Layer

```cpp
class UinputOutput {
public:
    static auto create(const Config& cfg) -> Result<UinputOutput>;
    auto emit(const GamepadState& state) -> Result<void>;
};
```

## Directory Structure

```
include/vader5/
├── types.hpp           # GamepadState, Result, constants
├── config.hpp          # TOML config
├── transport.hpp       # UsbTransport
├── protocol.hpp        # parse functions
├── output.hpp          # UinputOutput
└── device.hpp          # Device (combines all)

src/
├── transport.cpp
├── protocol.cpp
├── output.cpp
├── device.cpp
├── config.cpp
└── main.cpp
```

## Dependencies
- libusb-1.0 (USB access)
- linux/uinput.h (virtual device)
- linux/input.h (input events)
- toml++ (config, header-only)