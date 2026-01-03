# Change: Add Controller Key Remap

**Status: Future** - 硬件级别重映射，优先级低于用户空间重映射 (add-remap-system)

## Why
Vader 5 Pro supports onboard key remapping via USB commands. Extended buttons (C, Z, M1-M4, LM, RM, O) can be mapped to standard gamepad buttons, stored in controller firmware.

## What Changes
- Add Hidraw::write() for sending commands
- Implement Flydigi protocol commands (a4 06, a5 17)
- Add remap test in debug TUI

## Impact
- Affected specs: controller-config (new)
- Affected code: include/vader5/hidraw.hpp, src/hidraw.cpp, src/debug.cpp