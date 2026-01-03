# Configuration

vader5d uses TOML configuration files.

## File Location

```bash
# Default path
~/.config/vader5/config.toml

# Or specify custom path
sudo ./build/vader5d /path/to/config.toml
```

## Basic Settings

```toml
# Xbox Elite emulation (Steam paddle support)
emulate_elite = true

[gyro]
mode = "off"              # off / mouse / joystick
sensitivity = 1.5         # movement multiplier
sensitivity_x = 1.5       # horizontal (overrides sensitivity)
sensitivity_y = 1.5       # vertical (overrides sensitivity)
deadzone = 50             # ignore small movements
smoothing = 0.3           # 0.0-1.0, higher = smoother
curve = 1.0               # acceleration curve
invert_x = false
invert_y = false

[stick.left]
mode = "gamepad"          # gamepad / mouse / scroll
deadzone = 128
sensitivity = 1.0

[stick.right]
mode = "gamepad"          # gamepad / mouse / scroll
deadzone = 128
sensitivity = 1.0

[dpad]
mode = "gamepad"          # gamepad / arrows
```

## Layers (Mode Shift)

Hold a trigger button to activate a layer. Release to return to base mode.

```toml
[layer.name]
trigger = "LM"            # which button activates this layer
tap = "KEY_TAB"           # optional: key to send on quick tap
hold_timeout = 200        # ms before layer activates (for tap-hold)

# Override settings while layer is active
gyro = { mode = "mouse", sensitivity = 2.0 }
stick_left = { mode = "scroll" }
stick_right = { mode = "mouse", sensitivity = 1.5 }
dpad = { mode = "arrows" }

# Remap buttons while layer is active
remap = { RB = "mouse_left", RT = "mouse_right", A = "KEY_SPACE" }
```

### Trigger Buttons

Available triggers: `A`, `B`, `X`, `Y`, `LB`, `RB`, `LT`, `RT`, `M1`, `M2`, `M3`, `M4`, `LM`, `RM`, `C`, `Z`

### Tap-Hold Behavior

When `tap` is set:
- Quick press + release (< hold_timeout) = send tap key
- Hold (>= hold_timeout) = activate layer, no tap key sent

### Remap Targets

```toml
remap = {
  A = "KEY_SPACE",        # keyboard key
  B = "mouse_left",       # mouse button
  X = "mouse_right",
  Y = "mouse_middle",
  RB = "mouse_side",      # mouse button 4
  RT = "mouse_extra",     # mouse button 5
  LB = "disabled",        # disable button
}
```

## Button Remapping (Base Layer)

Remap extended buttons in base mode:

```toml
[remap]
M1 = "KEY_F13"
M2 = "KEY_F14"
M3 = "KEY_F15"
M4 = "KEY_F16"
C = "KEY_TAB"
Z = "KEY_ESC"
LM = "KEY_LEFTALT"
RM = "KEY_LEFTCTRL"
```

## Full Example

```toml
emulate_elite = true

[gyro]
mode = "off"

[stick.left]
deadzone = 128

[stick.right]
deadzone = 128

# Hold LM: gyro aim + full mouse mode
[layer.aim]
trigger = "LM"
tap = "mouse_side"
hold_timeout = 200
gyro = { mode = "mouse", sensitivity = 2.0 }
stick_left = { mode = "scroll" }
stick_right = { mode = "mouse", sensitivity = 1.0 }
dpad = { mode = "arrows" }
remap = { RB = "mouse_left", RT = "mouse_right", RM = "mouse_middle", A = "KEY_LEFTMETA" }

# Hold RM: stick mouse + arrows
[layer.mouse]
trigger = "RM"
hold_timeout = 150
stick_right = { mode = "mouse", sensitivity = 1.5 }
dpad = { mode = "arrows" }
remap = { A = "mouse_left", B = "mouse_right" }
```

## Key Codes Reference

Common key codes (full list in `/usr/include/linux/input-event-codes.h`):

| Key | Code |
|-----|------|
| A-Z | `KEY_A` - `KEY_Z` |
| 0-9 | `KEY_0` - `KEY_9` |
| F1-F24 | `KEY_F1` - `KEY_F24` |
| Space | `KEY_SPACE` |
| Enter | `KEY_ENTER` |
| Escape | `KEY_ESC` |
| Tab | `KEY_TAB` |
| Backspace | `KEY_BACKSPACE` |
| Left/Right/Up/Down | `KEY_LEFT`, `KEY_RIGHT`, `KEY_UP`, `KEY_DOWN` |
| Ctrl/Alt/Shift | `KEY_LEFTCTRL`, `KEY_LEFTALT`, `KEY_LEFTSHIFT` |
| Super (Win/Meta) | `KEY_LEFTMETA` |

## Mouse Buttons

| Button | Target |
|--------|--------|
| Left click | `mouse_left` |
| Right click | `mouse_right` |
| Middle click | `mouse_middle` |
| Side (back) | `mouse_side` |
| Extra (forward) | `mouse_extra` |