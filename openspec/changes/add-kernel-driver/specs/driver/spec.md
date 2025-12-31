## ADDED Requirements

### Requirement: Device Detection
The driver SHALL detect Flydigi Vader 5 Pro via USB (VID:0x37d7 PID:0x2401).

#### Scenario: Device connected
- **WHEN** 2.4G receiver plugged in
- **THEN** driver binds to device

### Requirement: Basic Buttons
The driver SHALL report standard gamepad buttons.

#### Scenario: Face buttons
- **WHEN** A/B/X/Y pressed
- **THEN** BTN_A/BTN_B/BTN_X/BTN_Y events emitted

#### Scenario: Shoulder buttons
- **WHEN** LB/RB pressed
- **THEN** BTN_TL/BTN_TR events emitted

#### Scenario: Menu buttons
- **WHEN** Start/Select pressed
- **THEN** BTN_START/BTN_SELECT events emitted

### Requirement: D-Pad
The driver SHALL report D-Pad as HAT axes.

#### Scenario: D-Pad directions
- **WHEN** D-Pad pressed
- **THEN** ABS_HAT0X/ABS_HAT0Y values -1/0/1

### Requirement: Analog Sticks
The driver SHALL report analog sticks with full range.

#### Scenario: Left stick
- **WHEN** left stick moved
- **THEN** ABS_X/ABS_Y range -32768 to 32767

#### Scenario: Right stick
- **WHEN** right stick moved
- **THEN** ABS_RX/ABS_RY range -32768 to 32767
