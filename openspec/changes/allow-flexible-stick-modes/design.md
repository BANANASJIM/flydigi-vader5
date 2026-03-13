# Design: Allow Flexible Stick Modes

## Current State

```
process_mouse_stick:  get_effective_stick_right() → state.right_x/right_y → move_mouse
process_scroll_stick: get_effective_stick_left()  → state.left_x/left_y   → scroll
```

Left stick cannot be mouse, right stick cannot be scroll.

## Target State

```
process_mouse_sticks:  for each stick in {left, right}:
                         if mode == Mouse → read that stick's axes → move_mouse

process_scroll_sticks: for each stick in {left, right}:
                         if mode == Scroll → read that stick's axes → scroll
```

## Implementation

### 1. Generalize process_mouse_stick

Process both sticks. Each stick in Mouse mode reads its own axes:

```cpp
void Gamepad::process_mouse_sticks(const GamepadState& state) {
    if (!input_) return;

    auto process = [&](const StickConfig& cfg, int sx, int sy) {
        if (cfg.mode != StickConfig::Mouse) return;
        if (std::abs(sx) < cfg.deadzone) sx = 0;
        if (std::abs(sy) < cfg.deadzone) sy = 0;
        int dx = static_cast<int>(static_cast<float>(sx) * STICK_SCALE * cfg.sensitivity);
        int dy = static_cast<int>(static_cast<float>(sy) * STICK_SCALE * cfg.sensitivity);
        if (dx != 0 || dy != 0) {
            input_->move_mouse(dx, dy);
            [[maybe_unused]] auto r = input_->sync();
        }
    };

    process(get_effective_stick_left(), state.left_x, state.left_y);
    process(get_effective_stick_right(), state.right_x, state.right_y);
}
```

### 2. Generalize process_scroll_stick

Same pattern, per-stick scroll accumulators:

```cpp
void Gamepad::process_scroll_sticks(const GamepadState& state) {
    if (!input_) return;

    auto process = [&](const StickConfig& cfg, int sx, int sy, float& av, float& ah) {
        if (cfg.mode != StickConfig::Scroll) { av = ah = 0.0F; return; }
        if (std::abs(sx) < cfg.deadzone) sx = 0;
        if (std::abs(sy) < cfg.deadzone) sy = 0;
        av += static_cast<float>(-sy) * SCROLL_SCALE * cfg.sensitivity;
        ah += static_cast<float>(sx) * SCROLL_SCALE * cfg.sensitivity;
        int sv = static_cast<int>(av), sh = static_cast<int>(ah);
        if (sv != 0 || sh != 0) {
            av -= static_cast<float>(sv);
            ah -= static_cast<float>(sh);
            input_->scroll(sv, sh);
            [[maybe_unused]] auto r = input_->sync();
        }
    };

    process(get_effective_stick_left(), state.left_x, state.left_y,
            scroll_accum_lv_, scroll_accum_lh_);
    process(get_effective_stick_right(), state.right_x, state.right_y,
            scroll_accum_rv_, scroll_accum_rh_);
}
```

### 3. State changes in gamepad.hpp

Replace single `scroll_accum_v_`/`scroll_accum_h_` with per-stick pairs:

```
- float scroll_accum_v_{0.0F};
- float scroll_accum_h_{0.0F};
+ float scroll_accum_lv_{0.0F};
+ float scroll_accum_lh_{0.0F};
+ float scroll_accum_rv_{0.0F};
+ float scroll_accum_rh_{0.0F};
```

### 4. Fix needs_mouse

```cpp
// base config: check both sticks for Mouse or Scroll
if (cfg.left_stick.mode == StickConfig::Mouse || cfg.left_stick.mode == StickConfig::Scroll)
    return true;
if (cfg.right_stick.mode == StickConfig::Mouse || cfg.right_stick.mode == StickConfig::Scroll)
    return true;

// layers: check both sticks for both modes
for (const auto& [name, layer] : cfg.layers) {
    for (const auto& s : {layer.stick_left, layer.stick_right}) {
        if (s && (s->mode == StickConfig::Mouse || s->mode == StickConfig::Scroll))
            return true;
    }
}
```
