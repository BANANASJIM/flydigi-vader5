# LED Control Implementation Tasks

## Phase 1: Core LED Support

- [x] Add LedConfig struct to config.hpp (mode, brightness, color)
- [x] Parse `[led]` section in config.cpp
- [x] Add LED state to Gamepad class
- [x] Implement HID command `a9 17` for LED control
- [x] Send LED command on startup
- [ ] **Verify LED protocol works on hardware** (blocked: no visible effect)

## Phase 2: Per-Layer LED

- [x] Add optional led field to LayerConfig
- [x] Apply layer LED on activation
- [x] Revert to base LED on deactivation
- [ ] **Verify layer LED switching works** (depends on Phase 1 verification)

## Phase 3: Dynamic Control (Optional)

- [ ] Create Unix socket listener
- [ ] Parse JSON commands
- [ ] Apply LED changes from socket

## Notes

Protocol debugging needed:
- Command `a9 17` implemented per research/protocol.md
- Sending mode packet (byte[4]=00) + color layer packet (byte[4]=01)
- Using GRB color format as documented
- hidraw write returns success but no visible LED change
- Need to verify: timing, packet order, or undocumented requirements