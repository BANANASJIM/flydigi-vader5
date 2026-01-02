## Tasks

### 1. Refactor to hidraw-only
- [x] Remove libusb/UsbTransport dependency
- [x] Use Interface 1 hidraw for all I/O
- [x] Send init sequence (01, a1, 02, 04) on open
- [x] Send test mode command (11 07) on open
- [x] Implement rumble via CMD 0x12 (5a a5 12 06 LL RR)

### 2. Extended Input
- [x] Parse extended report (5a a5 ef, 32 bytes)
- [x] Extract sticks, triggers, buttons from byte 3-16
- [x] Extract ext_buttons from byte 13-14
- [x] Forward BTN_TRIGGER_HAPPY1-9 to uinput
- [x] Map Home (EXT_HOME) to BTN_MODE

### 3. Code Quality
- [x] Create shared protocol.hpp for parsing utilities
- [x] Add comments for reverse-engineered commands
- [x] Eliminate code duplication (DPAD_MAP, read_s16)

### 4. Steam Input Config
- [x] Document SDL_GAMECONTROLLERCONFIG format for extra buttons
- [x] Create config/gamecontrollerdb.txt with full mapping
- [x] Xbox Elite emulation (VID 0x045e, PID 0x0b00)
- [x] M1-M4 mapped to BTN_TRIGGER_HAPPY5-8 (xpad Elite P1-P4)
- [ ] Test extended buttons in Steam Input controller config

### 5. Testing
- [ ] Verify all buttons with evtest
- [ ] Test rumble
- [ ] Test in actual game