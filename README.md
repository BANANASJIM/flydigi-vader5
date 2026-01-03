# Flydigi Vader 5 Pro Linux Driver

Linux userspace driver for the Flydigi Vader 5 Pro gamepad (2.4G USB dongle).

## Features

- Xbox Elite emulation with Steam paddle support (M1-M4)
- Gyro mouse / joystick mode
- Layer system with tap-hold (like QMK keyboard firmware)
- Button remap to keyboard/mouse

## Quick Start

```bash
git clone https://github.com/BANANASJIM/flydigi-vader5.git
cd flydigi-vader5
cmake -B build && cmake --build build
sudo ./build/vader5d
```

## How It Works

```
┌─────────────────────────────────────────────────────────┐
│                    Normal Mode                          │
│  Controller works as Xbox Elite, M1-M4 are paddles     │
└─────────────────────────────────────────────────────────┘
                          │
            Hold LM (< 200ms = tap, >= 200ms = hold)
                          ▼
┌─────────────────────────────────────────────────────────┐
│                   Layer: aim                            │
│  Gyro → mouse, RB → left click, RT → right click       │
└─────────────────────────────────────────────────────────┘
                          │
                    Release LM
                          ▼
                  Back to Normal
```

**Tap-hold**: Quick tap LM = mouse_side button, Hold LM = activate layer

## Configuration

Config: `config/config.toml`

```toml
emulate_elite = true

[gyro]
mode = "off"                # off / mouse / joystick

# Hold LM for gyro aim
[layer.aim]
trigger = "LM"
tap = "mouse_side"          # quick tap action
hold_timeout = 200          # ms
gyro = { mode = "mouse", sensitivity = 2.0 }
remap = { RB = "mouse_left", RT = "mouse_right" }

# Hold RM for stick mouse
[layer.mouse]
trigger = "RM"
stick_right = { mode = "mouse" }
dpad = { mode = "arrows" }

# Hold M4 for gyro-to-stick (games without gyro)
[layer.gyro_stick]
trigger = "M4"
gyro = { mode = "joystick" }
```

## Steam Paddles

| Vader 5 | Elite | Steam Input |
|---------|-------|-------------|
| M1 | P1 | Upper Left |
| M2 | P2 | Upper Right |
| M3 | P3 | Lower Left |
| M4 | P4 | Lower Right |

## Troubleshooting

```bash
# Permission denied
sudo cp install/99-vader5.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules

# Check controller
lsusb | grep 37d7

# Debug tool
sudo ./build/vader5-debug
```

## License

GPL-2.0