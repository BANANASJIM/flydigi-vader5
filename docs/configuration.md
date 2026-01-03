# Configuration

vader5d uses TOML configuration files.

## File Location

```bash
# Default (current directory)
./config.toml

# Custom path
sudo ./build/vader5d /path/to/config.toml
```

## Button Remapping

Map extended buttons to keyboard keys:

```toml
[remap]
M1 = "KEY_F13"
M2 = "KEY_F14"
M3 = "KEY_F15"
M4 = "KEY_F16"
C = "KEY_TAB"
Z = "KEY_ESC"
# O = "disabled"
```

Available buttons: `C`, `Z`, `M1`, `M2`, `M3`, `M4`, `LM`, `RM`, `O`

Key codes: Any Linux input key code (e.g., `KEY_A`, `KEY_SPACE`, `KEY_F13`)

## Gyroscope

```toml
[gyro]
mode = "off"              # off / mouse
sensitivity = 1.5
deadzone = 50
smoothing = 0.8
invert_x = false
invert_y = false
```

## Analog Sticks

```toml
[stick.left]
deadzone = 128

[stick.right]
deadzone = 128
as_mouse = false
mouse_sensitivity = 1.0
```

## Mode Shift

Hold a button to temporarily activate alternate modes:

```toml
[mode_shift.LM]
gyro = "mouse"            # Enable gyro-to-mouse
right_stick = "mouse"     # Right stick as mouse
RB = "mouse_left"         # RB as left click
RT = "mouse_right"        # RT as right click
```

Available mode shift buttons: `LM`, `RM`, `C`, `Z`

Mode shift options:
- `gyro` - `"off"` / `"mouse"`
- `right_stick` - `"mouse"` / `"scroll"`
- `left_stick` - `"scroll"`
- Button remaps - `"mouse_left"`, `"mouse_right"`, `"KEY_*"`

## Full Example

```toml
[remap]
M1 = "KEY_F13"
M2 = "KEY_F14"
C = "KEY_TAB"
Z = "KEY_ESC"

[gyro]
mode = "off"
sensitivity = 1.5
deadzone = 50

[stick.left]
deadzone = 128

[stick.right]
deadzone = 128

[mode_shift.LM]
gyro = "mouse"
right_stick = "mouse"
RB = "mouse_left"
RT = "mouse_right"
```
