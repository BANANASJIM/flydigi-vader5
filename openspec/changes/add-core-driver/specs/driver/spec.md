## ADDED Requirements

### Requirement: Device Discovery
The driver SHALL automatically discover Vader 5 Pro devices by USB VID:PID (0x37d7:0x2401).

#### Scenario: Device connected
- **WHEN** Vader 5 Pro is connected via USB or dongle
- **THEN** driver opens the correct hidraw interface

#### Scenario: Device not present
- **WHEN** no Vader 5 Pro is connected
- **THEN** driver waits for hotplug event

### Requirement: HID Report Parsing
The driver SHALL parse HID reports to extract all button and axis states.

#### Scenario: Standard buttons
- **WHEN** A/B/X/Y/LB/RB/Start/Select pressed
- **THEN** corresponding button events emitted

#### Scenario: Extended buttons
- **WHEN** M1/M2/M3/M4/C/Z buttons pressed
- **THEN** BTN_TRIGGER_HAPPY1-6 events emitted

#### Scenario: Analog axes
- **WHEN** joysticks or triggers moved
- **THEN** ABS_X/Y/RX/RY/Z/RZ events emitted with correct range

### Requirement: Virtual Device Creation
The driver SHALL create a virtual gamepad via uinput that exposes all inputs.

#### Scenario: Device appears in evtest
- **WHEN** driver starts successfully
- **THEN** new /dev/input/eventX device appears
- **AND** evtest shows all buttons and axes

### Requirement: Low Latency
The driver SHALL process input with less than 1ms latency.

#### Scenario: Button press timing
- **WHEN** button pressed on physical device
- **THEN** event appears on virtual device within 1ms
