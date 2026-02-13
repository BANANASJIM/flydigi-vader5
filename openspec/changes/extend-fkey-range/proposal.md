# Fix F11/F12 and Extend Function Key Range to F24

## Why

Two related issues with function key support:

1. **Bug — F11/F12 silently broken**: `InputDevice::create()` registers keys via two loops: `KEY_ESC`(1)–`KEY_KPDOT`(83) and `KEY_F13`(183)–`KEY_F24`(194). F11(87) and F12(88) fall in the gap, so they are never registered as uinput capabilities. Config parses them (they're in KEY_TABLE), but emitted events are silently dropped by the kernel.

2. **Gap — F17-F24 not in KEY_TABLE**: The InputDevice already registers F13-F24, but `keycodes.cpp` KEY_TABLE only has F1-F16, so config parsing rejects F17-F24. Docs claim F1-F24 support.

## What Changes

- `src/uinput.cpp`: Add F11/F12 to the explicit key registration list in `InputDevice::create()`
- `src/keycodes.cpp`: Add KEY_F17–KEY_F24 entries to KEY_TABLE

## Scope

Two-file fix. No changes to parsing logic, remap processing, or config structure.

## Risk

None — fixes a silent capability gap and adds missing lookup entries.
