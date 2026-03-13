# Remap System Spec Delta

## MODIFIED Requirements

### Requirement: Layer Input Modes

The driver SHALL support any stick in any input mode (mouse, scroll, gamepad), regardless of whether it is the left or right stick.

#### Scenario: Left stick mouse in layer
- **WHEN** layer contains `stick_left = { mode = "mouse" }`
- **THEN** left stick controls mouse cursor while layer active

#### Scenario: Right stick mouse in layer
- **WHEN** layer contains `stick_right = { mode = "mouse" }`
- **THEN** right stick controls mouse cursor while layer active

#### Scenario: Left stick scroll in layer
- **WHEN** layer contains `stick_left = { mode = "scroll" }`
- **THEN** left stick controls scroll wheel while layer active

#### Scenario: Right stick scroll in layer
- **WHEN** layer contains `stick_right = { mode = "scroll" }`
- **THEN** right stick controls scroll wheel while layer active

#### Scenario: Both sticks in non-gamepad modes
- **WHEN** layer contains `stick_left = { mode = "mouse" }` and `stick_right = { mode = "scroll" }`
- **THEN** left stick controls mouse cursor
- **AND** right stick controls scroll wheel
- **AND** both operate independently without interference
