# Motion Controls Research

## Steam Input Gyro Support

Steam Input 对已知控制器（DualShock 4, DualSense, Switch Pro, Steam Deck）有内置陀螺仪支持，通过 SDL/hidapi 直接读取。

**对于未知控制器（如 Vader 5 Pro）：**
- Steam Input 不会自动检测 evdev 暴露的陀螺仪
- SDL 只对已知控制器使用 hidapi 读取陀螺仪
- 需要其他方案

**可行方案：**
1. **DSU Server** - 用于模拟器（Dolphin, Cemu, Yuzu），Steam Input 本身不直接用 DSU
2. **uhid 虚拟 DualSense** - 让控制器看起来像 DualSense，SDL 会自动读取陀螺仪（复杂）
3. **evdev motion sensor** - 某些应用可能读取，但 Steam Input 不会

**结论：** 对于 Steam Input 陀螺仪支持，最可靠的方案是用 uhid 模拟已知控制器。DSU 只对模拟器有用。

---

# DSU Protocol Research

## Overview

DSU (DualShock UDP) protocol, also known as cemuhook protocol, is used to send motion data from controllers to emulators/games.

- Port: UDP 26760 (localhost)
- Protocol version: 1001
- Used by: Dolphin, Cemu, Yuzu, Citra, Ryujinx, Steam Input

## Packet Structure

All packets have a 16-byte header:

```cpp
struct Header {
    char magic[4];      // "DSUS" (server) or "DSUC" (client)
    uint16_t version;   // 1001
    uint16_t length;    // payload length (not including header)
    uint32_t crc32;     // CRC32 of entire packet (with this field = 0)
    uint32_t id;        // source ID, constant per session
    uint32_t eventType; // message type
};
```

## Message Types

| Type | Value | Description |
|------|-------|-------------|
| Version | 0x100000 | Protocol version info |
| Info | 0x100001 | Controller connection status |
| Data | 0x100002 | Controller input data |

## Controller Data Packet (0x100002)

```cpp
struct SharedResponse {
    uint8_t slot;       // controller slot 0-3
    uint8_t slotState;  // 0=disconnected, 1=reserved, 2=connected
    uint8_t deviceModel; // 0=N/A, 1=partial gyro, 2=full gyro
    uint8_t connection; // 0=N/A, 1=USB, 2=Bluetooth
    uint32_t mac1;      // unused, set to 0
    uint16_t mac2;      // unused, set to 0
    uint8_t battery;    // unused, set to 0
    uint8_t connected;  // 1=connected
};

struct MotionData {
    uint64_t timestamp; // microseconds
    float accX, accY, accZ;  // acceleration in g's
    float pitch, yaw, roll;  // gyro in deg/s
};

struct DataEvent {
    Header header;
    SharedResponse response;
    uint32_t packetNumber;
    // buttons...
    MotionData motion;
};
```

## Motion Data Format

- Accelerometer: X, Y, Z in g's (1g ≈ 9.8 m/s²)
- Gyroscope: pitch, yaw, roll in degrees/second
- Timestamp: 64-bit microseconds

## Key Implementation Notes

1. Server runs on UDP, no connection state
2. Client sends subscription request, server broadcasts data
3. Data rate: typically 100Hz
4. CRC32 uses polynomial 0xedb88320

## Reference Implementations

- [SteamDeckGyroDSU](https://github.com/kmicki/SteamDeckGyroDSU) - C++ implementation for Steam Deck
- [joycond-cemuhook](https://github.com/joaorb64/joycond-cemuhook) - Python for Joy-Con
- [PSMove-DSU](https://github.com/Swordmaster3214/PSMove-DSU) - C++ for PS Move

## Sources

- [Protocol Specification](https://v1993.github.io/cemuhook-protocol/)
- [Dolphin Wiki](https://wiki.dolphin-emu.org/index.php?title=DSU_Client)