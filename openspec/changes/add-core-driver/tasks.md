## 1. Project Setup
- [ ] 1.1 Create CMakeLists.txt with C++20 and clang-tidy
- [ ] 1.2 Create .clang-format and .clang-tidy configs
- [ ] 1.3 Create .gitignore

## 2. HID Reader
- [ ] 2.1 Implement hidraw device discovery (by VID/PID)
- [ ] 2.2 Implement hidraw file descriptor wrapper
- [ ] 2.3 Implement HID report parser for Vader 5 protocol

## 3. UInput Writer
- [ ] 3.1 Implement uinput device creation
- [ ] 3.2 Define button/axis mapping
- [ ] 3.3 Implement event emission

## 4. Daemon
- [ ] 4.1 Implement main event loop with epoll
- [ ] 4.2 Bridge hidraw events to uinput
- [ ] 4.3 Handle device hotplug (udev)

## 5. Testing
- [ ] 5.1 Manual test with evtest
- [ ] 5.2 Test with Steam Input
