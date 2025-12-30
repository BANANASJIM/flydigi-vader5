## Context

Vader 5 Pro has multiple HID interfaces:
- Interface 0: Vendor Specific
- Interface 1: Main input (20B) - buttons/sticks/triggers
- Interface 2: Extended input (32B) - M1-M4/IMU (test mode only)
- Interface 3-5: Keyboard/mouse emulation
- Interface 6: Control commands (hidraw)

## Goals / Non-Goals

**Goals:**
- Robust single-interface binding
- Debug-friendly logging
- Preserve other interfaces for kernel defaults

**Non-Goals:**
- Extended button support (requires test mode)
- IMU data processing

## Decisions

### Interface Selection: Only Interface 1

**Rationale:**
- Interface 1 has all standard gamepad inputs
- Extended buttons on Interface 2 only work in test mode
- Other interfaces should remain available for:
  - Keyboard/mouse emulation (kernel HID)
  - Configuration tool (hidraw on Interface 6)

### Debug Logging Strategy

| Level | Usage |
|-------|-------|
| `hid_dbg()` | Per-report details, skipped interfaces |
| `hid_info()` | Successful init/remove |
| `hid_err()` | Failures with error codes |

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| Extended buttons unavailable | Document test mode requirement |
| Interface number assumptions | Log actual interface numbers |
