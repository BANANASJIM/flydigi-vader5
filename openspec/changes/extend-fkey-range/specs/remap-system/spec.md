# remap-system Spec Delta

## MODIFIED Requirements

### Requirement: Base Button Remapping

All function keys F1-F24 SHALL be recognized as valid remap targets and correctly emitted via InputDevice.

#### Scenario: Remap to F11 (previously broken)
- **WHEN** config contains `M1 = "KEY_F11"` in `[remap]`
- **AND** M1 is pressed
- **THEN** KEY_F11 emits via InputDevice
- **AND** original gamepad button is suppressed

#### Scenario: Remap to F12 (previously broken)
- **WHEN** config contains `M2 = "KEY_F12"` in `[remap]`
- **AND** M2 is pressed
- **THEN** KEY_F12 emits via InputDevice

#### Scenario: Remap to extended function key F20
- **WHEN** config contains `M3 = "KEY_F20"` in `[remap]`
- **AND** M3 is pressed
- **THEN** KEY_F20 emits via InputDevice

#### Scenario: Layer remap to extended function key
- **WHEN** layer config contains `remap = { M4 = "KEY_F17" }`
- **AND** layer is active and M4 is pressed
- **THEN** KEY_F17 emits via InputDevice

#### Scenario: Invalid key name rejected
- **WHEN** config contains `M1 = "KEY_F25"`
- **THEN** config load logs error for unrecognized key name
- **AND** remap entry is skipped
