## Tasks

### 1. Userspace Testing
- [x] 1.1 Add UsbTransport::write()
- [x] 1.2 Add rumble test in debug TUI
- [x] 1.3 Test Xbox 360 format (EP5 OUT) - works
- [x] 1.4 Test Xbox One format - NOT supported by controller

### 2. Driver Implementation
- [x] 2.1 Add output URB and buffer allocation
- [x] 2.2 Find EP5 OUT in probe()
- [x] 2.3 Register FF_RUMBLE effect
- [x] 2.4 Implement ff_play callback
- [x] 2.5 Add work queue for rumble packets

### 3. Testing
- [ ] 3.1 Test with fftest
- [ ] 3.2 Test with SDL game
- [ ] 3.3 Verify cleanup on disconnect