# Flydigi Vader 5 Pro Linux Driver

Linux userspace driver for the Flydigi Vader 5 Pro gamepad (2.4G USB dongle).

[![License: GPL-2.0](https://img.shields.io/badge/License-GPL%202.0-blue.svg)](LICENSE)

## Features

- **Xbox Elite emulation** - Appears as Xbox Elite Series 2 with paddle support
- **Full button support** - All standard Xbox buttons + extended buttons (C, Z, M1-M4, LM, RM, O)
- **Analog sticks & triggers** - Full 16-bit resolution
- **Rumble/vibration** - Xbox 360 compatible force feedback
- **Gyro-to-mouse** - Use gyroscope as mouse with configurable sensitivity and response curve
- **Mode shift** - Hold a button to activate mouse mode, D-pad arrows, scroll wheel
- **Button remapping** - Remap extended buttons to keyboard keys or mouse buttons
- **Steam Input compatible** - Works seamlessly with Steam Input's Xbox Elite paddle configuration

## Steam Input Paddle Support

This driver emulates an **Xbox Elite Series 2** controller, which Steam Input recognizes natively. The extended buttons are mapped to Elite paddle buttons:

| Vader 5 Button | Elite Paddle | Steam Input |
|----------------|--------------|-------------|
| M1 | P1 | Upper Left Paddle |
| M2 | P2 | Upper Right Paddle |
| M3 | P3 | Lower Left Paddle |
| M4 | P4 | Lower Right Paddle |

In Steam, go to **Controller Settings** → select your controller → **Configure** to bind these paddles to any action.

## Requirements

- Linux kernel 5.0+
- CMake 3.20+
- C++23 compiler (GCC 13+ or Clang 17+)

Dependencies (toml++, ftxui, libusb) are automatically downloaded via CMake.

## Installation

### Quick Install
```bash
git clone https://github.com/BANANASJIM/flydigi-vader5.git
cd flydigi-vader5
./install/install.sh
```

This will build the project, install udev rules, and create a config file.

### Full System Install
```bash
./install/install.sh install
```

This additionally installs binaries to `/usr/local/bin` and creates a systemd service.

### Manual Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cp install/99-vader5.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
```

## Usage

### Run the driver daemon
```bash
# Direct run (foreground)
sudo ./build/vader5d

# Or with custom config
sudo ./build/vader5d /path/to/config.toml

# If installed system-wide
sudo systemctl start vader5d
```

### Debug tool
```bash
sudo ./build/vader5-debug
```

Shows real-time input state including buttons, sticks, triggers, and IMU data.

## Configuration

Config file location: `~/.config/vader5/config.toml`

### Button Remapping

Remap extended buttons to keyboard keys or mouse buttons:

```toml
[remap]
M1 = "KEY_F13"      # Map M1 to F13
M2 = "KEY_F14"
M3 = "mouse_left"   # Map M3 to left mouse button
M4 = "mouse_right"
C = "KEY_TAB"
Z = "KEY_ESC"
O = "disabled"      # Disable O button
```

Available targets:
- Keyboard: `KEY_A` through `KEY_Z`, `KEY_F1` through `KEY_F16`, `KEY_ESC`, `KEY_TAB`, `KEY_SPACE`, etc.
- Mouse: `mouse_left`, `mouse_right`, `mouse_middle`
- Disable: `disabled`

### Gyro Mouse

Use the controller's gyroscope as a mouse:

```toml
[gyro]
mode = "mouse"      # off / mouse
sensitivity = 1.5
deadzone = 50
smoothing = 0.8
invert_x = false
invert_y = false
```

### Mode Shift

Temporarily activate features while holding a button:

```toml
[mode_shift.LM]            # When holding LM button
gyro = "mouse"             # Enable gyro mouse
right_stick = "mouse"      # Right stick controls mouse
RB = "mouse_left"          # RB becomes left click
RT = "mouse_right"         # RT becomes right click
dpad = "arrows"            # D-pad becomes arrow keys
left_stick = "scroll"      # Left stick becomes scroll wheel
```

### Stick Configuration

```toml
[stick.left]
deadzone = 128

[stick.right]
deadzone = 128
as_mouse = false           # Use right stick as mouse
mouse_sensitivity = 1.0
```

## Project Structure

```
flydigi-vader5/
├── src/                    # Userspace driver source
│   ├── daemon/             # vader5d daemon
│   └── tools/              # Debug and utility tools
├── include/vader5/         # Public headers
├── driver/                 # Kernel driver (WIP)
├── install/                # Installation scripts
├── config/                 # Default configuration
└── docs/                   # Documentation
```

## Troubleshooting

### Permission denied
```bash
# Install udev rules
sudo cp install/99-vader5.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
# Replug the USB dongle
```

### Controller not detected
- Ensure the 2.4G USB dongle is plugged in
- Check if the controller is paired with the dongle
- Verify with `lsusb | grep 37d7`

### Steam doesn't detect paddles
- Make sure Steam is running after starting vader5d
- In Steam: Settings → Controller → Enable Xbox Extended Feature Support

## Roadmap

- [x] Basic input (buttons, sticks, triggers)
- [x] Rumble support (Xbox 360 format)
- [x] Debug TUI tool
- [x] Extended buttons (test mode)
- [x] IMU/Gyro data
- [x] Gyro-to-mouse
- [x] Mode shift
- [x] Button remapping
- [ ] Kernel HID driver
- [ ] LED control

## Contributing

Contributions welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) first.

## License

GPL-2.0 - See [LICENSE](LICENSE) for details.

## Acknowledgments

- [vader3](https://github.com/ahungry/vader3) - Initial protocol research
- [flydigictl](https://github.com/pipe01/flydigictl) - Bluetooth protocol reference
- [xpadneo](https://github.com/atar-axis/xpadneo) - Xbox controller driver reference