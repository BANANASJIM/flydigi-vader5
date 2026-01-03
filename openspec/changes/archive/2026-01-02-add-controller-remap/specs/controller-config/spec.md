## ADDED Requirements

### Requirement: Hidraw Write
The Hidraw class SHALL support writing commands to the controller.

#### Scenario: Send command via hidraw
- **WHEN** a 64-byte command is written
- **THEN** the command is sent to the controller via hidraw

### Requirement: Test Mode
The system SHALL send test mode command (11 07) to enable extended button reporting.

#### Scenario: Enable test mode
- **WHEN** test mode command is sent with enable=true
- **THEN** controller starts sending extended button reports on Interface 2

#### Scenario: Disable test mode
- **WHEN** test mode command is sent with enable=false
- **THEN** controller stops sending extended button reports

### Requirement: Extended Button Display
The debug TUI SHALL display extended button states when test mode is enabled.

#### Scenario: Display extended buttons
- **WHEN** test mode is enabled
- **THEN** TUI shows M1-M4, C, Z, LM, RM, O button states

### Requirement: Key Remap Command
The system SHALL send Flydigi key remap commands (a4 06) to configure extended button mappings.

#### Scenario: Remap M1 to A button
- **WHEN** remap command is sent with src=M1(2), target=A(04)
- **THEN** controller maps M1 press to A button

#### Scenario: Clear mapping
- **WHEN** remap command is sent with target=ff
- **THEN** controller removes the button mapping

### Requirement: Save Configuration
The system SHALL send save command (a6 04) to persist mappings to controller storage.

#### Scenario: Save after remap
- **WHEN** save command is sent
- **THEN** mappings are stored in controller flash