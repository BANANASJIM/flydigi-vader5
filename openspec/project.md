# Project Context

## Purpose
Linux userspace driver for Flydigi Vader 5 Pro gamepad, enabling:
- Extended button support (M1-M4 back buttons, C/Z buttons)
- Gyroscope data
- Key remapping
- Steam Input compatibility

## Tech Stack
- C++20
- CMake 3.20+
- libevdev / libudev
- Linux uinput subsystem

## Project Conventions

### Code Style
- clang-format with LLVM style
- clang-tidy enabled with modernize-* and cppcoreguidelines-*
- Headers: declaration only, no implementation
- Minimal comments, self-explanatory code
- No checkout operations in git

### Architecture Patterns
- Single-threaded event loop with epoll
- RAII for resource management
- std::expected for error handling (C++23) or custom Result type
- Minimal dependencies

### Testing Strategy
- Unit tests with Catch2
- Integration tests with mock hidraw device

### Git Workflow
- Conventional commits: feat/fix/refactor/docs
- Feature branches: feature/<name>
- Main branch protected

## Domain Context
- HID (Human Interface Device) protocol
- Linux input subsystem (evdev, uinput)
- USB VID:PID for Vader 5 Pro: 0x37d7:0x2401
- Reference protocol from flydigictl (Go) and vader3 (kernel driver)

## Important Constraints
- Must run as root or with uinput/hidraw permissions
- Must not conflict with xpad kernel driver
- Low latency (<1ms processing time)

## External Dependencies
- /dev/hidraw* for raw HID data
- /dev/uinput for virtual device creation
