# remap-system Specification

## Purpose
TBD - created by archiving change add-remap-system. Update Purpose after archive.
## Requirements
### Requirement: Elite Emulation Mode

The driver SHALL support `emulate_elite` config option to control gamepad type and base remap behavior.

#### Scenario: Elite emulation enabled (default)
- **WHEN** config contains `emulate_elite = true` or option is absent
- **THEN** create virtual gamepad as Xbox Elite Series 2 (0x045e:0x0b00)
- **AND** Steam Input detects paddles (M1-M4 = P1-P4)
- **AND** base `[remap]` section is ignored (buttons pass to gamepad)

#### Scenario: Elite emulation disabled
- **WHEN** config contains `emulate_elite = false`
- **THEN** create virtual gamepad as standard (0x37d7:0x2401)
- **AND** base `[remap]` section is active
- **AND** remapped buttons emit to InputDevice, not gamepad

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

### Requirement: Layer Definition

The driver SHALL support layer definitions via `[layer.Name]` sections with trigger and optional overrides.

#### Scenario: Layer config structure
- **WHEN** config contains `[layer.Gaming]` with `trigger = "RM"`
- **THEN** hold RM activates layer "Gaming"
- **AND** layer can have inline overrides: `gyro = { mode = "mouse" }`, `remap = { A = "mouse_left" }`
- **AND** undefined settings inherit from base config

#### Scenario: Layer activation
- **WHEN** layer trigger button is held
- **THEN** layer becomes active
- **AND** layer remaps take effect

#### Scenario: Layer deactivation
- **WHEN** layer trigger button is released
- **THEN** layer becomes inactive
- **AND** layer remaps stop

### Requirement: Single-Layer Mode

The driver SHALL support single-layer mode where only one layer is active at a time.

#### Scenario: First layer takes priority
- **WHEN** layer "aim" is activated (trigger held >= timeout)
- **AND** user presses another layer trigger
- **THEN** second layer does NOT activate
- **AND** second layer trigger can be remapped by active layer

#### Scenario: Layer inheritance
- **WHEN** base [remap] has M1 = KEY_F13
- **AND** [layer.RM.remap] does not define M1
- **THEN** M1 inherits base mapping (KEY_F13) while layer RM active

### Requirement: Conflict Detection

The driver SHALL detect and log mapping conflicts at config load time.

#### Scenario: Same button in multiple layers
- **WHEN** button A is mapped in both [layer.RM] and [layer.LM]
- **THEN** log warning that both will emit when both layers active

#### Scenario: Trigger button also remapped
- **WHEN** button RM is layer trigger
- **AND** button RM is also in `[remap]`
- **THEN** log error about conflict
- **AND** ignore base remap for trigger button

#### Scenario: Base remap overridden by layer
- **WHEN** button M1 remapped in `[remap]`
- **AND** button M1 also remapped in a layer
- **THEN** log info that layer overrides base when active

### Requirement: Mode Shift Backward Compatibility

The driver SHALL accept `[mode_shift.X]` as alias for `[layer.X]` with deprecation warning.

#### Scenario: Mode shift config loaded
- **WHEN** config contains `[mode_shift.LM]`
- **THEN** treat as `[layer.LM]` with trigger = "LM"
- **AND** log deprecation warning recommending `[layer.Name]` format

### Requirement: Tap-Hold Layer Trigger

The driver SHALL support tap-hold behavior for layer triggers (home row mod style).

#### Scenario: Tap trigger button
- **WHEN** layer trigger is pressed and released within hold_timeout (default 200ms)
- **THEN** emit tap action if configured
- **AND** layer does NOT activate

#### Scenario: Hold trigger button
- **WHEN** layer trigger is held for >= hold_timeout
- **THEN** layer activates
- **AND** tap action does NOT emit

#### Scenario: Hold then release
- **WHEN** layer trigger is held >= hold_timeout then released
- **THEN** layer deactivates
- **AND** no tap action emits

#### Scenario: Tap-hold config
- **WHEN** layer contains `tap = "KEY_F13"` and `hold_timeout = 200`
- **THEN** quick tap emits KEY_F13
- **AND** hold >= 200ms activates layer

### Requirement: Layer Input Modes

The driver SHALL support keyboard, mouse, and gyro modes within layers.

#### Scenario: Gyro mouse in layer
- **WHEN** layer contains `gyro = { mode = "mouse" }`
- **THEN** gyro controls mouse cursor while layer active

#### Scenario: Gyro joystick in layer
- **WHEN** layer contains `gyro = { mode = "joystick" }`
- **THEN** gyro values map to right stick axes while layer active
- **AND** useful for games without native gyro support

#### Scenario: Right stick mouse in layer
- **WHEN** layer contains `right_stick = "mouse"`
- **THEN** right stick controls mouse cursor while layer active

#### Scenario: Left stick scroll in layer
- **WHEN** layer contains `left_stick = "scroll"`
- **THEN** left stick controls scroll wheel while layer active

#### Scenario: Dpad arrows in layer
- **WHEN** layer contains `dpad = "arrows"`
- **THEN** dpad emits KEY_UP/DOWN/LEFT/RIGHT while layer active

#### Scenario: Keyboard remap in layer
- **WHEN** layer contains `A = "KEY_SPACE"`
- **THEN** A emits KEY_SPACE via InputDevice while layer active
- **AND** A does NOT emit BTN_SOUTH to gamepad

### Requirement: Layer Remap Priority

The driver SHALL give layer remaps priority over base remaps, emitting only the layer target when both define the same button.

#### Scenario: Layer remap overrides base remap
- **WHEN** base `[remap]` has `M1 = "KEY_F13"`
- **AND** active layer has `remap = { M1 = "mouse_left" }`
- **AND** M1 is pressed
- **THEN** only BTN_LEFT emits (from layer)
- **AND** KEY_F13 does NOT emit (base skipped)
- **AND** BTN_TRIGGER_HAPPY5 does NOT emit (suppressed)

