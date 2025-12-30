## Tasks

### 1. Driver Modifications
- [x] 1.1 Add interface number check in flydigi_probe()
- [x] 1.2 Add hid_info/err logging to probe/remove
- [x] 1.3 Remove flydigi_parse_ext() and extended button code
- [x] 1.4 Create manual input device (flydigi_input_create)
- [x] 1.5 Set drvdata BEFORE hid_hw_start (race condition fix)
- [x] 1.6 Use HID_CONNECT_DEFAULT (enables raw_event callback)
- [x] 1.7 Add hid_hw_open()/close() to receive HID reports
- [x] 1.8 Rewrite as USB driver (not HID) for Interface 0

### 2. Issues Investigated
- [x] 2.1 raw_event not called with HID_CONNECT_HIDRAW only
- [x] 2.2 raw_event not called with HID_CONNECT_HIDRAW | HID_CONNECT_DRIVER
- [x] 2.3 drvdata was NULL in raw_event (set after hid_hw_start)
- [x] 2.4 flydigi_input_create overwrote drvdata
- [x] 2.5 Interface 0 is vendor-specific (Xbox subclass 0x5d), not HID
- [x] 2.6 Main input is on EP1 IN (32 bytes) via Interface 0

### 3. Testing
- [x] 3.1 Build driver with make
- [x] 3.2 Load driver and check dmesg logs
- [ ] 3.3 Verify raw_event receives data (check dmesg for "flydigi: size=")
- [ ] 3.4 Verify evtest shows correct input
- [ ] 3.5 Verify hidraw interfaces remain accessible

### 4. Validation
- [x] 4.1 Run openspec validate fix-driver-interface-filter --strict