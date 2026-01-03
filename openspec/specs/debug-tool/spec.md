# Debug Tool Specification

## Purpose

TUI diagnostic tool for Vader 5 Pro controller. Displays real-time input state, IMU data, and provides rumble testing.

## Requirements

### Requirement: TUI Display

The tool SHALL display real-time controller state in a terminal UI using ftxui.

#### Scenario: Display controller state
- Given vader5-debug is running
- When controller input changes
- Then display updates in real-time
- And shows sticks, triggers, buttons, d-pad

### Requirement: Test Mode Support

The tool SHALL support test mode for extended input and IMU data.

#### Scenario: Enable test mode
- Given user presses T key
- When test mode is enabled
- Then extended buttons (C,Z,M1-M4,LM,RM,O,Home) are displayed
- And IMU data (gyro X/Y/Z, accel X/Y/Z) is displayed

### Requirement: Rumble Testing

The tool SHALL support rumble testing via keyboard.

#### Scenario: Rumble controls
- Given user presses number keys
- When 1-9 pressed -> set both motors to intensity
- When 0 pressed -> stop rumble
- When Z pressed -> left motor only
- When X pressed -> right motor only
- When C pressed -> both motors full