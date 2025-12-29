# Add Key Mapping

## Summary
Add configurable key mapping for extended buttons (M1-M4, C, Z, Circle, Home).

## Features
- TOML config file (~/.config/vader5/config.toml)
- Map extended buttons to keyboard keys or gamepad buttons
- Runtime config reload (SIGHUP)

## Config Example
```toml
[mappings]
M1 = "KEY_F1"
M2 = "KEY_F2"
M3 = "KEY_F3"
M4 = "KEY_F4"
C = "KEY_ENTER"
Z = "KEY_ESC"

[options]
deadzone = 4000
```

## Implementation
1. Add toml++ (header-only)
2. Create config parser
3. Extend Uinput for keyboard EV_KEY
4. Apply mappings in emit()
