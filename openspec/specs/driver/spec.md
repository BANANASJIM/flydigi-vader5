# Driver Specification

## Purpose

Userspace driver for Flydigi Vader 5 Pro gamepad on Linux. Provides virtual gamepad via uinput with extended button support.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Vader 5 Pro Hardware                     │
│                   (VID 0x37d7, PID 0x2401)                  │
└─────────────────────────────────────────────────────────────┘
                              │
                    USB HID (Interface 1)
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      vader5d (daemon)                        │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   Hidraw    │  │  Protocol   │  │      Uinput         │  │
│  │  Interface  │──│   Parser    │──│ Virtual Gamepad     │  │
│  │             │  │  (5a a5 ef) │  │ (Elite emulation)   │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                              │
                         /dev/uinput
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Linux Input Subsystem                     │
│              Vader 5 Pro Virtual Gamepad                     │
│          (emulates Xbox Elite Series 2)                      │
└─────────────────────────────────────────────────────────────┘
```

## Requirements

### Requirement: HID Communication

The driver SHALL communicate with the controller via hidraw Interface 1 in test mode.

#### Scenario: Controller initialization
- Given the controller is connected via USB
- When vader5d starts
- Then it sends init sequence (01, a1, 02, 04)
- And it sends test mode command (11 07)
- And it reads extended reports (5a a5 ef)

### Requirement: Extended Report Parsing

The driver SHALL parse 32-byte extended reports with magic bytes 5a a5 ef.

#### Scenario: Parse extended report
- Given an extended report is received
- When the magic bytes are 5a a5 ef
- Then sticks are parsed from bytes 3-10 (int16 LE)
- And buttons are parsed from bytes 11-12
- And ext_buttons are parsed from bytes 13-14
- And triggers are parsed from bytes 15-16
- And IMU data is parsed from bytes 17-28

### Requirement: Xbox Elite Emulation

The driver SHALL emulate Xbox Elite Series 2 controller (VID 0x045e, PID 0x0b00) via uinput.

#### Scenario: Create virtual gamepad
- Given vader5d starts successfully
- When uinput device is created
- Then VID is 0x045e and PID is 0x0b00
- And device name is "Vader 5 Pro Virtual Gamepad"

### Requirement: Standard Button Mapping

The driver SHALL map standard gamepad buttons to Linux input codes.

#### Scenario: Standard button mapping
- Given button state changes
- When A is pressed -> BTN_SOUTH
- When B is pressed -> BTN_EAST
- When X is pressed -> BTN_NORTH
- When Y is pressed -> BTN_WEST
- When LB is pressed -> BTN_TL
- When RB is pressed -> BTN_TR
- When L3 is pressed -> BTN_THUMBL
- When R3 is pressed -> BTN_THUMBR
- When Start is pressed -> BTN_START
- When Select is pressed -> BTN_SELECT

### Requirement: Extended Button Mapping

The driver SHALL map extended buttons to BTN_TRIGGER_HAPPY codes matching xpad Elite paddles.

#### Scenario: Extended button mapping
- Given ext_buttons state changes
- When C is pressed -> BTN_TRIGGER_HAPPY1
- When Z is pressed -> BTN_TRIGGER_HAPPY2
- When M1 is pressed -> BTN_TRIGGER_HAPPY5 (Elite P1)
- When M2 is pressed -> BTN_TRIGGER_HAPPY6 (Elite P2, hardware bit 4)
- When M3 is pressed -> BTN_TRIGGER_HAPPY7 (Elite P3, hardware bit 3)
- When M4 is pressed -> BTN_TRIGGER_HAPPY8 (Elite P4)
- When LM is pressed -> BTN_TRIGGER_HAPPY3
- When RM is pressed -> BTN_TRIGGER_HAPPY4
- When O is pressed -> BTN_TRIGGER_HAPPY9
- When Home is pressed -> BTN_MODE

### Requirement: Analog Input

The driver SHALL forward analog inputs with correct ranges and dead zones.

#### Scenario: Stick input
- Given stick position changes
- When left/right stick moves
- Then emit ABS_X/Y and ABS_RX/RY
- And range is -32768 to 32767
- And Y axis is inverted (up = positive)
- And flat zone is 128, fuzz is 16

#### Scenario: Trigger input
- Given trigger position changes
- When LT/RT is pressed
- Then emit ABS_Z/RZ
- And range is 0 to 255
- And no dead zone

### Requirement: Rumble Support

The driver SHALL support Xbox 360 style rumble via hidraw CMD 0x12.

#### Scenario: Send rumble
- Given rumble request received
- When left/right motor values set
- Then send packet: 5a a5 12 06 LL RR