# Change: Add Extended Input Support

## Why
Steam Input cannot see extended buttons (C, Z, M1-M4, LM, RM, O) or gyro data because they are only available via test mode on Interface 1. Need to expose these through standard Linux input subsystem and DSU protocol.

## What Changes

### Phase 1: Userspace (vader5d)
- Send init + test mode commands on startup
- Parse extended input report (5a a5 ef) from hidraw
- Create uinput device with BTN_TRIGGER_HAPPY1-10
- Built-in DSU server for Steam Input gyro support

### Phase 2: Kernel Driver (future)
- Move extended input handling to hid-flydigi.ko
- No daemon needed for basic functionality

## Impact
- Affected specs: extended-input (new)
- Affected code: src/daemon/main.cpp, src/uinput.cpp, new src/dsu.cpp