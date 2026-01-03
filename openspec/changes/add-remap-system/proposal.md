# Change: Add Multi-Layer Remap System

## Why

Current remap functionality has limitations:
1. Button remaps only work in mode_shift context - cannot remap buttons to keyboard/mouse without holding a modifier
2. Mode shift only supports single layer - cannot stack multiple layers
3. No conflict detection when same button mapped multiple ways
4. Remap and Elite emulation are coupled - cannot use one without the other

Users need flexible remapping that works independently and supports layered configurations for complex workflows (e.g., productivity, gaming macros).

## What Changes

- Add `emulate_elite` option: when false, standard gamepad + all buttons can remap
- **BREAKING**: Rename `[mode_shift.X]` to `[layer.X]` for clarity
- Add `[remap]` section processing for direct button-to-key/mouse mapping (without layer activation)
- Add multi-layer support: layers can be nested and combined
- Add layer priority system for conflict resolution
- Add **tap-hold** for layer triggers (home row mod style): tap = key, hold = layer
- Add conflict detection with warning logs
- Base remaps only active when `emulate_elite = false`

## Impact

- Affected specs: `driver`, `extended-input`, new `remap-system`
- Affected code:
  - `src/gamepad.cpp` - add `process_button_remaps()`, layer state machine
  - `src/config.cpp` - parse new layer config format
  - `include/vader5/config.hpp` - layer structures
  - `config/config.toml` - updated example config

## Migration

Existing `[mode_shift.X]` configs continue to work but emit deprecation warning. Recommend migrating to `[layer.X]` format.