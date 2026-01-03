## Context

Vader 5 Pro has extended buttons and IMU only accessible via test mode:
- Interface 0: Standard Xbox input (EP1 IN) + rumble (EP5 OUT) - not needed
- Interface 1: Config/Extended (EP2 IN, EP6 OUT) - hidraw, provides ALL functionality
- Extended report format: `5a a5 ef` + sticks/buttons/triggers/gyro/accel

## Interface 1 Commands (EP6 OUT, 32 bytes)

| Command | Format | Description |
|---------|--------|-------------|
| Init | `5a a5 01 02 03`, `a1 02 a3`, `02 02 04`, `04 02 06` | Handshake sequence |
| Test Mode ON | `5a a5 11 07 ff 01 ff ff ff 15 00...` | Enable extended input |
| Test Mode OFF | `5a a5 11 07 ff 00 ff ff ff 14 00...` | Disable extended input |
| **Rumble** | `5a a5 12 06 LL RR 00...` | LL=left, RR=right (0-255) |

Steam Input supports:
- Linux evdev for buttons/sticks
- DSU protocol (UDP 26760) for motion controls

## Goals / Non-Goals

Goals:
- Extended buttons visible in Steam Input
- Gyro working in Steam/emulators

Non-Goals:
- LED control (separate feature)
- Button remapping (handled by Steam)

## Decisions

### 1. Phase 1: Userspace driver (vader5d)
- Send init + test mode commands via hidraw
- Parse extended report from EP2
- Create uinput device with extended buttons
- Built-in DSU server for gyro

Rationale: Faster iteration, easier debugging, no kernel rebuild.

### 2. Phase 2: Kernel driver (future)
- Move to hid-flydigi.ko for system-wide support
- No daemon needed for basic functionality

### 3. DSU built into vader5d
- Single daemon handles everything
- No separate vader5-dsu tool needed

Rationale: Simpler deployment, shared gamepad state.

## Architecture (Phase 1: Userspace)

```
┌─────────────────────────────────────────────────────────┐
│                     vader5d (hidraw only)               │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │              Interface 1 (hidraw)                 │  │
│  │   EP6 OUT: init, test mode, rumble commands       │  │
│  │   EP2 IN:  extended input (32B, 5a a5 ef)         │  │
│  └───────────────────────────────────────────────────┘  │
│                          │                              │
│                          ▼                              │
│  ┌───────────────────────────────────────────────────┐  │
│  │              Gamepad State                        │  │
│  │  - sticks (16-bit), triggers, buttons             │  │
│  │  - extended buttons (C,Z,M1-4,LM,RM,O,Home)       │  │
│  │  - gyro, accel (6-axis)                           │  │
│  └───────────────────────────────────────────────────┘  │
│                          │                              │
│                          ▼                              │
│  ┌───────────────────────────────────────────────────┐  │
│  │         uhid DualSense (virtual controller)       │  │
│  │  - all buttons + extended as paddles              │  │
│  │  - gyro/accel in DualSense format                 │  │
│  │  - rumble feedback via uhid                       │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
                           │
                           ▼
                    Steam Input / SDL
                  (full gyro + buttons)
```

Benefits:
- No libusb dependency, pure hidraw
- Single interface for all I/O
- DualSense emulation = native Steam gyro support

## Button Mapping

| Extended Button | Linux Key Code |
|-----------------|----------------|
| C | BTN_TRIGGER_HAPPY1 |
| Z | BTN_TRIGGER_HAPPY2 |
| M1 | BTN_TRIGGER_HAPPY3 |
| M2 | BTN_TRIGGER_HAPPY4 |
| M3 | BTN_TRIGGER_HAPPY5 |
| M4 | BTN_TRIGGER_HAPPY6 |
| LM | BTN_TRIGGER_HAPPY7 |
| RM | BTN_TRIGGER_HAPPY8 |
| O | BTN_TRIGGER_HAPPY9 |
| Home | BTN_MODE |

Note: Home button is the center wake button, maps to MODE for Xbox compatibility.

## Gyro Mapping (evdev)

| IMU Axis | evdev | Range |
|----------|-------|-------|
| Gyro X | ABS_RX | -32768..32767 |
| Gyro Y | ABS_RY | -32768..32767 |
| Gyro Z | ABS_RZ | -32768..32767 |
| Accel X | ABS_TILT_X | -32768..32767 |
| Accel Y | ABS_TILT_Y | -32768..32767 |

## DSU Protocol

Port: 26760 (default cemuhook port)
Broadcast: 100Hz
Format: See https://v1993.github.io/cemern-docs/protocol/

## Open Questions

- Should DSU server be part of this driver or separate project?
- Need to test if Steam actually reads BTN_TRIGGER_HAPPY for controller config