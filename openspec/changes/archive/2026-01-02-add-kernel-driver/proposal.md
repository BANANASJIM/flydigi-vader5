# Change: Add Kernel HID Driver

## Why
Replace userspace driver with kernel HID driver for:
- No daemon required
- Auto-load on device connect
- Lower latency
- Better Steam Input compatibility

## What Changes
- Add `driver/hid-flydigi.c` kernel module
- Add Makefile for kbuild
- Add DKMS configuration
- Add udev rules

## Impact
- Affected specs: driver
- Affected code: driver/
