# Change: Allow Flexible Stick Modes

## Why

`process_mouse_stick` hardcodes right stick, `process_scroll_stick` hardcodes left stick.
Users cannot configure `stick_left = { mode = "mouse" }` or `stick_right = { mode = "scroll" }`.
This was reported by user xl666 on PR #22.

Additionally, `needs_mouse()` layer checks are incomplete: only checks right stick for Mouse and left stick for Scroll, missing the cross combinations.

## What Changes

- Generalize `process_mouse_stick` / `process_scroll_stick` to accept either stick
- Fix `needs_mouse()` to check all stick+mode combinations in both base config and layers
- Add per-stick scroll accumulators to support simultaneous configurations
- Update remap-system spec to reflect both sticks supporting all modes

## Impact

- Affected code: `src/gamepad.cpp`, `include/vader5/gamepad.hpp`
- Modified spec: `remap-system` (Layer Input Modes requirement)
- Users can now freely assign mouse/scroll mode to either stick
