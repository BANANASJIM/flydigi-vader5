## ADDED Requirements

### Requirement: Toggle Activation Mode

The driver SHALL support toggle activation mode for layers, where tap activates and tap again deactivates.

#### Scenario: Toggle layer on
- **WHEN** layer contains `activation = "toggle"`
- **AND** trigger button is tapped
- **THEN** layer activates and remains active after release

#### Scenario: Toggle layer off
- **WHEN** toggle layer is active
- **AND** trigger button is tapped again
- **THEN** layer deactivates

#### Scenario: Toggle mode single-layer constraint
- **WHEN** toggle layer is active
- **AND** another layer trigger is pressed
- **THEN** second layer does NOT activate