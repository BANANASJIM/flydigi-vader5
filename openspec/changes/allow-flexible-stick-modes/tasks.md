# Tasks

## 1. Fix needs_mouse detection
- [x] 1.1 In `needs_mouse()`, add `cfg.right_stick.mode == StickConfig::Scroll` to base config check
- [x] 1.2 In `needs_mouse()` layer loop, check both sticks for both Mouse and Scroll modes

## 2. Generalize stick processing
- [x] 2.1 Refactor `process_mouse_stick` to process both sticks in Mouse mode using their respective axis data
- [x] 2.2 Refactor `process_scroll_stick` to process both sticks in Scroll mode using their respective axis data

## 3. State management
- [x] 3.1 Split `scroll_accum_v_`/`scroll_accum_h_` into per-stick pairs in `gamepad.hpp` to avoid interference when both sticks have non-gamepad modes

## 4. Validation
- [x] 4.1 Build and run existing tests (`test-remap`, `test-uinput-elite`, `test-debug-iface`)
- [ ] 4.2 Verify with `evtest` that `stick_left = { mode = "mouse" }` moves cursor
- [ ] 4.3 Verify with `evtest` that `stick_right = { mode = "scroll" }` scrolls
