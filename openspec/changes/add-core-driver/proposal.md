# Change: Add Core Vader 5 Pro Driver

## Why
Flydigi Vader 5 Pro gamepad's extended buttons (M1-M4, C/Z) are not recognized by the default xpad kernel driver. A userspace driver is needed to expose all inputs to games and Steam Input.

## What Changes
- Add hidraw reader to capture raw HID data from the gamepad
- Add uinput writer to create virtual gamepad with all buttons
- Add daemon process to bridge hidraw → uinput
- Support VID 0x37d7, PID 0x2401

## Impact
- Affected specs: driver (new)
- Affected code: src/, include/
