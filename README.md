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
- **Steam Input compatible** - Separate mouse device prevents Steam detection issues

## Supported Devices

| Device | VID:PID | Connection | Status |
|--------|---------|------------|--------|
| Vader 5 Pro | 37d7:2401 | 2.4G USB | ✅ Supported |

## Requirements

- Linux kernel 5.0+
- libusb 1.0
- CMake 3.20+
- C++23 compiler (GCC 13+ or Clang 17+)

## Quick Start

```bash
# Clone
git clone https://github.com/user/flydigi-vader5.git
cd flydigi-vader5

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install udev rules (for non-root access)
sudo cp install/99-vader5.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules

# Run daemon
sudo ./build/vader5d

# Or run debug TUI
sudo ./build/vader5-debug
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
└── openspec/               # Design specifications
```

## Configuration

Edit `config.toml` in the project directory, or pass a custom path:

```bash
sudo ./build/vader5d /path/to/config.toml
```

See [config.toml](config.toml) for all options.

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