# Implementation Tasks

## 1. Config Layer

- [x] 1.1 Add `emulate_elite` option to Config (default true)
- [x] 1.2 Add `LayerConfig` struct (tap, hold_timeout, remap, gyro, stick, dpad overrides)
- [x] 1.3 Parse `[layer.Name]` sections with trigger field
- [x] 1.4 Parse inline table config: `gyro = { mode = "mouse" }`
- [x] 1.5 Implement layer inheritance from base config
- [x] 1.6 Add backward compat for `[mode_shift.X]` with deprecation warning
- [x] 1.7 Add conflict detection with log warnings

## 2. Uinput Refactor

- [x] 2.1 Layer trigger buttons suppressed in base processing
- [x] 2.2 Add all mouse button codes (side, extra, forward, back)

## 3. Base Remap Processing

- [x] 3.1 Add `process_base_remaps()` for `[remap]` section
- [x] 3.2 Only process base remaps when `emulate_elite = false`
- [x] 3.3 Update `needs_mouse()` to check base button_remaps

## 4. Tap-Hold State Machine

- [x] 4.1 Add TapHoldState struct with layer_name, press_time, layer_activated
- [x] 4.2 Track press timestamp per layer trigger
- [x] 4.3 Implement hold_timeout detection (default 200ms)
- [x] 4.4 Emit tap action on quick release
- [x] 4.5 Activate layer on timeout

## 5. Layer Processing

- [x] 5.1 `get_active_layer()` returns single active layer (first activated wins)
- [x] 5.2 `get_effective_*()` returns layer override or base config
- [x] 5.3 `process_layer_buttons()` via InputDevice
- [x] 5.4 Process layer gyro/stick/dpad modes
- [x] 5.5 Add layer activation logging

## 6. Config & Docs

- [x] 6.1 Update `config/config.toml` with layer examples
- [x] 6.2 Add tap-hold examples
- [x] 6.3 Update README with layer documentation

## 7. Gyro Joystick Mode

- [x] 7.1 Add `GyroConfig::Joystick` mode
- [x] 7.2 Map gyro to right stick axes with JOYSTICK_SCALE

## 8. Code Quality

- [x] 8.1 Fix stick Y-axis overflow in protocol parsing
- [x] 8.2 Update .clang-tidy for toml++ compatibility
- [x] 8.3 Remove NOLINT comments, use project .clang-tidy