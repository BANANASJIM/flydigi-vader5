# Tasks

## 1. Uinput FF Support
- [ ] 1.1 Change uinput open mode from `O_WRONLY` to `O_RDWR`
- [ ] 1.2 Register `EV_FF` and `FF_RUMBLE` capabilities
- [ ] 1.3 Set `ff_effects_max` in uinput_setup
- [ ] 1.4 Implement `poll_ff()` to handle FF upload/play events
- [ ] 1.5 Store uploaded effects in `ff_effects_[]` array

## 2. Gamepad Integration
- [ ] 2.1 Add `ff_fd()` method to expose uinput fd
- [ ] 2.2 Add `poll_ff()` wrapper that calls uinput and triggers `send_rumble()`

## 3. Main Loop
- [ ] 3.1 Poll both hidraw and uinput fds
- [ ] 3.2 Call `poll_ff()` when uinput has data

## 4. Validation
- [ ] 4.1 Build and run vader5d
- [ ] 4.2 Test with `fftest /dev/input/eventX` or SDL rumble test
