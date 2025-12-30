## ADDED Requirements

### Requirement: Force Feedback Support
The driver SHALL support rumble/vibration via the Linux force feedback API.

#### Scenario: Rumble effect
- **WHEN** a FF_RUMBLE effect is played
- **THEN** the driver sends a rumble packet to EP5 OUT
- **AND** left/right motor intensity is mapped from effect magnitude

#### Scenario: Stop rumble
- **WHEN** the effect is stopped or device closes
- **THEN** the driver sends zero intensity to both motors
