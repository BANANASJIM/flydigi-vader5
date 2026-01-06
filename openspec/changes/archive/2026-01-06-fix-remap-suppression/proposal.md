# Fix Remap Button Suppression

## Why

The remap system had critical implementation gaps:
- Remapped buttons still emit original gamepad events (duplicate input)
- `disabled` type has no effect
- Layer remaps don't override base remaps correctly

## What Changes

- Add button suppression in `src/gamepad.cpp` and `include/vader5/gamepad.hpp`
- Track suppressed buttons per-frame using bitset
- Handle `RemapTarget::Disabled` to suppress without emitting
- Layer remaps override base remaps (not additive)