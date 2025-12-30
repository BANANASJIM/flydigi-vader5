# Change: Fix HID Driver Interface Filter

## Why

The current driver binds to all USB interfaces, causing:
- Multiple input devices created
- Driver data overwritten per interface
- Other interfaces (keyboard/mouse emulation, hidraw config) broken

## What Changes

- Add interface number check in probe()
- Only bind to Interface 1 (main input)
- Add debug logging throughout
- Remove unused extended button code (test mode only)

## Impact

- Affected specs: hid-driver
- Affected code: driver/hid-flydigi.c
