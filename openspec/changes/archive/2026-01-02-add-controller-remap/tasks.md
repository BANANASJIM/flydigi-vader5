## Tasks

### 1. Hidraw Write Support
- [x] 1.1 Change Hidraw::open() to O_RDWR
- [x] 1.2 Add Hidraw::write() method

### 2. Test Mode Support
- [x] 2.1 Add test mode command constants
- [x] 2.2 Add send_test_mode() function
- [x] 2.3 Add ext_input_thread() for Interface 2
- [x] 2.4 Add config_thread() for commands
- [x] 2.5 Fix ExtButton enum in types.hpp

### 3. Debug TUI Integration
- [x] 3.1 Add [T] key to toggle test mode
- [x] 3.2 Add render_ext_buttons() display
- [x] 3.3 Show test mode status in title

### 4. Remap Commands (TODO)
- [ ] 4.1 Implement send_remap_command()
- [ ] 4.2 Implement send_save_command()
- [ ] 4.3 Test M1->A, M2->B remapping