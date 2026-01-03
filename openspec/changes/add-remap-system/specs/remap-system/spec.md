## ADDED Requirements

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

The driver SHALL support direct button remapping via `[remap]` section without requiring layer activation.

#### Scenario: Remap button to keyboard key
- **WHEN** config contains `M1 = "KEY_F13"` in `[remap]`
- **THEN** pressing M1 emits KEY_F13 via InputDevice
- **AND** M1 does NOT emit BTN_TRIGGER_HAPPY5 to gamepad

#### Scenario: Remap button to mouse button
- **WHEN** config contains `M2 = "mouse_left"` in `[remap]`
- **THEN** pressing M2 emits BTN_LEFT via InputDevice
- **AND** M2 does NOT emit BTN_TRIGGER_HAPPY6 to gamepad

#### Scenario: All mouse buttons supported
- **WHEN** remap target is mouse button
- **THEN** support: mouse_left, mouse_right, mouse_middle, mouse_side, mouse_extra, mouse_forward, mouse_back

#### Scenario: Disable button
- **WHEN** config contains `C = "disabled"` in `[remap]`
- **THEN** pressing C emits nothing
- **AND** C is suppressed in all layers

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

### Requirement: Multi-Layer Support

The driver SHALL support multiple layers active simultaneously with merged mappings.

#### Scenario: Two layers active
- **WHEN** [layer.RM] maps A to KEY_1
- **AND** [layer.LM] maps B to KEY_2
- **AND** both RM and LM are held
- **THEN** A emits KEY_1 and B emits KEY_2 (both layers merge)

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
- **WHEN** layer contains `gyro = "mouse"`
- **THEN** gyro controls mouse cursor while layer active

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