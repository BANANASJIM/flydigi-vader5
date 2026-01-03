# Change: Add Rumble/Force Feedback Support

## Why

Interface 0 has EP5 OUT (32B) which may support Xbox-style rumble commands.
Games using SDL/Steam Input expect force feedback via the input subsystem.

## What Changes

- Add force feedback (ff) support to the USB driver
- Test Xbox-style rumble format on EP5 OUT
- Fallback to `5a a5 12 06` format if Xbox format doesn't work

## Impact

- Affected code: driver/hid-flydigi.c
- New feature: FF_RUMBLE support