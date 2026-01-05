## MODIFIED Requirements

### Requirement: Base Button Remapping

The driver SHALL suppress remapped buttons from emitting to the virtual gamepad, ensuring only the remap target is emitted.

#### Scenario: Remapped button suppressed from gamepad
- **WHEN** button M1 is remapped to KEY_F13 in `[remap]`
- **AND** M1 is pressed
- **THEN** KEY_F13 emits via InputDevice
- **AND** BTN_TRIGGER_HAPPY5 does NOT emit to virtual gamepad

#### Scenario: Disabled button fully suppressed
- **WHEN** button C is set to `disabled` in `[remap]`
- **AND** C is pressed
- **THEN** no event emits to InputDevice
- **AND** no event emits to virtual gamepad
- **AND** C remains suppressed in all layers

### Requirement: Layer Remap Priority

The driver SHALL give layer remaps priority over base remaps, emitting only the layer target when both define the same button.

#### Scenario: Layer remap overrides base remap
- **WHEN** base `[remap]` has `M1 = "KEY_F13"`
- **AND** active layer has `remap = { M1 = "mouse_left" }`
- **AND** M1 is pressed
- **THEN** only BTN_LEFT emits (from layer)
- **AND** KEY_F13 does NOT emit (base skipped)
- **AND** BTN_TRIGGER_HAPPY5 does NOT emit (suppressed)