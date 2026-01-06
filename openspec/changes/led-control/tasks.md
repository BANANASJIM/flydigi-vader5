# LED Control Implementation Tasks

## Phase 1: Core LED Support

- [ ] Add LedConfig struct to config.hpp (mode, brightness, color)
- [ ] Parse `[led]` section in config.cpp
- [ ] Add LED state to Gamepad class
- [ ] Implement HID command `a9 17` for LED control
- [ ] Send LED command on startup

## Phase 2: Per-Layer LED

- [ ] Add optional led field to LayerConfig
- [ ] Apply layer LED on activation
- [ ] Revert to base LED on deactivation

## Phase 3: Dynamic Control (Optional)

- [ ] Create Unix socket listener
- [ ] Parse JSON commands
- [ ] Apply LED changes from socket