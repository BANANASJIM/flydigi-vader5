## ADDED Requirements

### Requirement: LED Configuration

The driver SHALL support LED configuration via `[led]` section.

#### Scenario: Static LED mode
- **WHEN** config contains `[led]` with `mode = "static"`
- **THEN** LED displays solid color at specified brightness

#### Scenario: LED off
- **WHEN** config contains `mode = "off"`
- **THEN** LED is disabled

#### Scenario: Breathing mode
- **WHEN** config contains `mode = "breathing"`
- **THEN** LED pulses between off and specified color

### Requirement: Per-Layer LED

The driver SHALL support per-layer LED configuration.

#### Scenario: Layer LED override
- **WHEN** layer contains `led = { mode = "...", color = "..." }`
- **AND** layer activates
- **THEN** LED changes to layer config

#### Scenario: Layer LED revert
- **WHEN** layer with LED config deactivates
- **THEN** LED reverts to base config
