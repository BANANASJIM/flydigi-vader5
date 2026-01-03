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
    └─── Hold >= 200ms ─────► Layer activates (no tap emit)
                                   │
                              Gyro → mouse
                              RB → left click
                              RT → right click
                                   │
                              Release LM → Layer deactivates
```

### Layer System

```
┌─────────────────────────────────────────────────────────┐
│                    Base (Normal)                        │
│  Xbox Elite gamepad, M1-M4 = Steam paddles             │
└─────────────────────────────────────────────────────────┘
        │                              │
     Hold LM                        Hold RM
        ▼                              ▼
   ┌──────────────┐              ┌──────────────┐
   │     aim      │              │    mouse     │
   │ gyro + mouse │              │ stick mouse  │
   │ scroll + pad │              │   + arrows   │
   └──────────────┘              └──────────────┘

Only one layer active at a time (first activated wins)
```

## Configuration

Config: `config/config.toml`

```toml
emulate_elite = true        # true: Xbox Elite (Steam paddles), false: standard gamepad

[gyro]
mode = "off"                # off / mouse / joystick

# Hold LM for gyro aim + full mouse mode
[layer.aim]
trigger = "LM"              # trigger button: A/B/X/Y/LB/RB/LT/RT/M1-M4/LM/RM/C/Z
tap = "mouse_side"          # tap action: KEY_*, mouse_left/right/middle/side/extra
hold_timeout = 200          # ms before layer activates
gyro = { mode = "mouse", sensitivity = 2.0 }
stick_left = { mode = "scroll" }   # mode: gamepad / mouse / scroll
stick_right = { mode = "mouse", sensitivity = 1.0 }  # mode: gamepad / mouse
dpad = { mode = "arrows" }  # mode: gamepad / arrows / scroll
remap = { RB = "mouse_left", RT = "mouse_right", RM = "mouse_middle", A = "KEY_LEFTMETA" }

# Hold RM for stick mouse + arrows
[layer.mouse]
trigger = "RM"
stick_right = { mode = "mouse", sensitivity = 1.5 }
dpad = { mode = "arrows" }
remap = { A = "mouse_left", B = "mouse_right" }
```

See [docs/configuration.md](docs/configuration.md) for full options.

## Steam Paddles

| Vader 5 | Elite | Steam Input  |
|---------|-------|--------------|
| M1      | P1    | Upper Left   |
| M2      | P2    | Upper Right  |
| M3      | P3    | Lower Left   |
| M4      | P4    | Lower Right  |

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

## Roadmap

- [ ] Kernel driver (hid-flydigi) for native support
- [ ] LED/lighting control
- [ ] Onboard profile configuration (write to controller)
- [ ] Bluetooth support
- [ ] Trigger deadzone and curve settings
- [ ] Macro support

## License

GPL-2.0