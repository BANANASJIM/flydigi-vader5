# Remap System Design

## Context

The current implementation has `ModeShiftConfig` which activates alternative input modes when a trigger button is held. However:
- Direct remaps (`config_.button_remaps`) are parsed but never processed
- All remaps require holding a modifier button
- Only one mode shift can be active at a time

Users want:
1. Direct remaps without modifiers (e.g., M1 always emits KEY_F13)
2. Layer stacking (e.g., hold RM for layer1, then also hold LM for layer2)
3. Different layers can have keyboard, mouse, or gyro mappings

## Goals

- Direct button remaps work without any layer activation
- Single-layer mode: only one layer active at a time (first activated wins)
- Active layer can remap other layer triggers
- Conflict detection logs warnings at startup
- Backward compatible with existing mode_shift configs

## Non-Goals

- Macro support (key sequences, delays)
- Per-game profiles (use systemd --user instances instead)
- GUI configuration

## Tap-Hold (Home Row Mods)

Like QMK/ZMK keyboard firmware, support dual-function buttons:

- **Tap** (< hold_timeout) → emit tap action
- **Hold** (>= hold_timeout) → activate layer, no tap action

```toml
[layer.Gaming]
trigger = "RM"               # hold RM to activate layer
tap = "KEY_F13"              # quick tap → KEY_F13
hold_timeout = 200           # ms (default 200)
gyro = { mode = "mouse" }
remap = { A = "mouse_left" }
```

### Tap-Hold State Machine

```
Button Press
     │
     ▼
┌─────────────┐
│   PENDING   │ ← start timer
└─────────────┘
     │
     ├──────────────────────────────────────┐
     │ released < timeout                   │ held >= timeout
     ▼                                      ▼
┌─────────────┐                      ┌─────────────┐
│  EMIT TAP   │                      │ ACTIVATE    │
│  (KEY_F13)  │                      │   LAYER     │
└─────────────┘                      └─────────────┘
                                           │
                                           │ released
                                           ▼
                                     ┌─────────────┐
                                     │ DEACTIVATE  │
                                     │   LAYER     │
                                     └─────────────┘
```

### Tap-Hold Modes

| Mode | Behavior |
|------|----------|
| `hold-preferred` (default) | Wait for timeout before deciding |
| `tap-preferred` | Emit tap immediately, layer on re-press within timeout |
| `balanced` | Tap if released before another key, else hold |

## Architecture

### Layer Model

Like QMK/ZMK keyboards - simple layer activation from base:

```
┌─────────────────────────────────────────────────────────────┐
│                      Base Layer                              │
│  [remap] section - always active when emulate_elite=false   │
└─────────────────────────────────────────────────────────────┘
        │ hold RM              │ hold LM
        ▼                      ▼
┌──────────────────┐   ┌──────────────────┐
│   Layer: RM      │   │   Layer: LM      │
│   (gyro mouse)   │   │   (scroll mode)  │
└──────────────────┘   └──────────────────┘
        │ release              │ release
        └──────────┬───────────┘
                   ▼
            Back to Base
```

- Base → Layer only (no layer-to-layer jumps)
- **Single-layer mode**: Only one layer active at a time
- First activated layer blocks other layer triggers
- Active layer can remap other layer's trigger buttons

### Config Structure

Layers inherit from base and only override what's defined. All layer config in one block using TOML inline tables:

```toml
# Device mode
emulate_elite = false    # false = standard gamepad + remaps
                         # true = Elite emulation (for Steam), base remaps disabled

# ═══════════════════════════════════════════════════════════════
# Base config - defaults for all layers
# ═══════════════════════════════════════════════════════════════

[remap]
A = "KEY_SPACE"
B = "KEY_E"
M1 = "KEY_F13"
M2 = "mouse_left"

[gyro]
mode = "off"
sensitivity = 1.5

[stick.left]
deadzone = 4000

[stick.right]
mode = "gamepad"
deadzone = 4000

# ═══════════════════════════════════════════════════════════════
# Layers - [layer.Name] format
# ═══════════════════════════════════════════════════════════════

[layer.Gaming]
trigger = "RM"                     # hold RM to activate
tap = "KEY_F13"                    # quick tap emits KEY_F13
hold_timeout = 200                 # ms before layer activates
gyro = { mode = "mouse", sensitivity = 2.0 }
stick_right = { mode = "mouse" }
remap = { A = "mouse_left", B = "mouse_right" }

[layer.Scroll]
trigger = "LM"
tap = "KEY_F14"
stick_left = { mode = "scroll" }
dpad = { mode = "arrows" }
```

### Two Operating Modes

| Feature | `emulate_elite = true` | `emulate_elite = false` |
|---------|------------------------|-------------------------|
| Virtual gamepad | Elite Series 2 (0x045e:0x0b00) | Standard gamepad (0x37d7:0x2401) |
| Steam paddle | Works (M1-M4 = P1-P4) | Not detected |
| Base `[remap]` | Disabled (buttons → gamepad) | Enabled (buttons → keyboard/mouse) |
| Layer remaps | Works | Works |
| Use case | Steam gaming with paddles | General use with custom remaps |

### Conflict Detection

At config load time, detect conflicts:

1. **Same button in multiple layers**: Log warning (both will trigger if both layers active)
2. **Trigger button also remapped in [remap]**: Log error, ignore base remap
3. **Base remap overridden by layer**: Log info

Example log output:
```
[INFO] Layer 'Gaming' activated (trigger: RM)
[WARN] Button A mapped in 'Gaming' and 'Scroll' - both emit when active together
[ERROR] Button RM is layer trigger for 'Gaming', ignoring [remap] for RM
[INFO] Button M1: [remap]=KEY_F13, 'Gaming'=mouse_left - layer overrides when active
```

## Decisions

### Decision: [layer.Name] subtables
Each layer is a subtable under `[layer]` with the layer name as key. Cleaner than array syntax.

**Migration**: Accept `[mode_shift.X]` as `[layer.X]` with trigger=X.

### Decision: Single-layer mode
Only one layer can be active at a time. First activated layer takes priority. This simplifies conflict resolution and allows active layer to remap other layer triggers.

### Decision: Base remaps via InputDevice
Base remaps (from `[remap]` section) output to InputDevice (keyboard/mouse), not to the gamepad Uinput. This allows remapping without affecting gamepad behavior.

**Rationale**: If user remaps M1 to KEY_F13, they probably want a keyboard key, not a gamepad button change.

## Processing Flow

```cpp
void Gamepad::poll() {
    // 1. Read HID report
    auto state = parse_report(buf);

    // 2. Process gyro (respects active layers)
    process_gyro(state);

    // 3. Process stick mouse/scroll (respects active layers)
    process_mouse_stick(state);
    process_scroll_stick(state);

    // 4. Process base remaps (always active)
    process_button_remaps(state, prev_state_);

    // 5. Process layer remaps (when layer trigger held)
    process_layer_buttons(state, prev_state_);

    // 6. Process dpad arrows (in active layers)
    process_layer_dpad(state, prev_state_);

    // 7. Emit gamepad state (buttons not handled by remaps)
    uinput_.emit(state, prev_state_);
}
```

### Button Suppression

When a button is remapped (base or layer), it should NOT also emit the default gamepad event. Implementation:

```cpp
// Track which buttons are handled by remaps
std::bitset<32> suppressed_buttons_;

// In emit():
if (!suppressed_buttons_.test(button_index)) {
    emit_key(code, pressed);
}
```

## Risks / Trade-offs

### Risk: Conflicting layer mappings
If user maps same button in multiple layers, both will emit when both layers active.
**Mitigation**: Warn at config load time. User should avoid mapping same button in multiple layers.

### Risk: Tap-hold timing misfires
Quick taps during fast gameplay might accidentally activate layer instead of tap action.
**Mitigation**: Configurable `hold_timeout` (default 200ms). User can tune per-layer.

## Open Questions

1. Should disabled buttons in base remap also be suppressed in layers?
   - Answer: Yes, base `disabled` prevents all output for that button

2. ~~Should layers inherit from base remap or override completely?~~
   - Resolved: Layers inherit from base, only override what's explicitly defined