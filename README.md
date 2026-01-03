# Flydigi Vader 5 Pro Linux Driver

Linux userspace driver for the Flydigi Vader 5 Pro gamepad (2.4G USB dongle).

## Features

- Xbox Elite emulation with Steam paddle support (M1-M4)
- Gyro support: mouse mode or map to right stick (for games without gyro)
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

### Tap-Hold (Home Row Mods)

Like QMK/ZMK keyboard firmware - one button, two functions:

```
Press LM
    │
    ├─── Release < 200ms ───► Tap: emit mouse_side
    │
    └─── Hold >= 200ms ─────► Layer activates
                                   │
                              Gyro → mouse
                              RB → left click
                              RT → right click
                                   │
                              Release LM
                                   │
                              Layer deactivates
```

### Layer System

```
┌─────────────────────────────────────────────────────────┐
│                    Base (Normal)                        │
│  Xbox Elite gamepad, M1-M4 = Steam paddles             │
└─────────────────────────────────────────────────────────┘
        │                    │                    │
     Hold LM              Hold RM              Hold M4
        ▼                    ▼                    ▼
   ┌─────────┐          ┌─────────┐          ┌─────────┐
   │  aim    │          │  mouse  │          │gyro_stick│
   │gyro aim │          │stick    │          │gyro→stick│
   └─────────┘          │mouse    │          └─────────┘
                        └─────────┘

Only one layer active at a time (first activated wins)
```

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