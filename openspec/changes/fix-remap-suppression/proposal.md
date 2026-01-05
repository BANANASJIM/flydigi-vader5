# Fix Remap Button Suppression

## Problem

The remap system (from `add-remap-system`) has critical implementation gaps:

1. **Button suppression not implemented** - Remapped buttons still emit original gamepad events
2. **`disabled` type not handled** - Buttons configured as `disabled` have no effect
3. **Base/layer remap conflict** - Same button in both sends duplicate events
4. **Layer trigger skip too strict** - Trigger buttons skipped even when layer inactive

## Impact

- Games receive duplicate input (keyboard + gamepad) for remapped buttons
- `disabled` buttons still function normally
- Layer override of base remaps doesn't work correctly

## Solution

Implement button suppression as specified in design.md (lines 225-237):
- Track suppressed buttons per-frame using bitset
- Skip suppressed buttons in `uinput_.emit()`
- Handle `RemapTarget::Disabled` to suppress without emitting
- Layer remaps override base remaps (not additive)

## Scope

- `src/gamepad.cpp` - Add suppression logic
- `include/vader5/gamepad.hpp` - Add suppressed_buttons_ member
- No config changes required