# Tasks

1. [x] Fix F11/F12 registration: add KEY_F11, KEY_F12 to explicit key list in `InputDevice::create()` (`src/uinput.cpp:305`)
2. [x] Add KEY_F17–KEY_F24 entries to KEY_TABLE in `src/keycodes.cpp`
3. [x] Build passes
4. [ ] Verify: config with `M1 = "KEY_F12"` emits correct keycode (no longer silent failure)
5. [ ] Verify: config with `M2 = "KEY_F20"` loads and emits correctly
