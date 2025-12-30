## Tasks

### 1. Research
- [ ] 1.1 用 vader5-debug 测试 Xbox 格式 (EP5 OUT)
- [ ] 1.2 用 vader5-debug 测试 Flydigi 格式 (EP6 OUT)
- [ ] 1.3 确认两种格式的马达响应差异

### 2. Implementation
- [ ] 2.1 Add output URB and buffer allocation
- [ ] 2.2 Find EP5 OUT in probe()
- [ ] 2.3 Register FF_RUMBLE effect
- [ ] 2.4 Implement ff_play callback
- [ ] 2.5 Add work queue for rumble packets

### 3. Testing
- [ ] 3.1 Test with fftest
- [ ] 3.2 Test with SDL game
- [ ] 3.3 Verify cleanup on disconnect
