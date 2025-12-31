## ADDED Requirements

### Requirement: Real-time Input Display
The system SHALL display gamepad input state in real-time.

#### Scenario: Button press visualization
- **WHEN** user presses a button on the gamepad
- **THEN** the corresponding button in the TUI is highlighted

#### Scenario: Stick position display
- **WHEN** user moves an analog stick
- **THEN** the stick position is visualized graphically

#### Scenario: Trigger pressure display
- **WHEN** user presses a trigger
- **THEN** a progress bar shows the pressure level (0-100%)

### Requirement: Button Mapping Configuration
The system SHALL allow users to remap extended buttons to standard gamepad buttons.

#### Scenario: Map extended button
- **WHEN** user selects source button (C, Z, M1-M4, LM, RM, O)
- **AND** user selects target button (A, B, X, Y, LB, RB, etc.)
- **THEN** the mapping command is sent to the gamepad
- **AND** the mapping is saved to onboard storage

#### Scenario: Clear button mapping
- **WHEN** user selects "None" as target
- **THEN** the source button mapping is cleared

### Requirement: Profile Management
The system SHALL support switching between 4 configuration profiles.

#### Scenario: Switch profile
- **WHEN** user selects profile 1-4
- **THEN** the profile switch command is sent
- **AND** the UI updates to show current profile

### Requirement: Vibration Configuration
The system SHALL allow users to configure motor vibration strength.

#### Scenario: Set vibration strength
- **WHEN** user adjusts left/right motor strength (0-100%)
- **THEN** the vibration setting is saved to current profile

#### Scenario: Test vibration
- **WHEN** user triggers vibration test
- **THEN** the motors activate with current settings

### Requirement: LED Configuration
The system SHALL allow users to configure LED lighting effects.

#### Scenario: Set LED mode
- **WHEN** user selects LED mode (solid, breathing, gradient, cycle, flow, off)
- **THEN** the LED mode is applied

#### Scenario: Set LED brightness
- **WHEN** user adjusts brightness (0-100)
- **THEN** the brightness setting is applied

#### Scenario: Set LED color
- **WHEN** user selects RGB color
- **THEN** the color is applied to the LEDs
