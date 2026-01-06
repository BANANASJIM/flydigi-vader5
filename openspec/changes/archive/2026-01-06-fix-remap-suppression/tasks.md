# Tasks

1. [x] Add `suppressed_buttons_` and `suppressed_ext_` members to Gamepad class
2. [x] Create `button_to_masks()` helper for button name → mask mapping
3. [x] Implement suppression in `process_base_remaps()`:
   - Mark remapped buttons in suppressed masks
   - Handle `RemapTarget::Disabled` (suppress only, no emit)
   - Skip if layer remap takes priority
4. [x] Implement suppression in `process_layer_buttons()`:
   - Mark remapped buttons in suppressed masks
5. [x] Apply suppression in `poll()`:
   - Clear suppressed masks at start
   - Mask out suppressed buttons before emit
6. [x] Build passes with clang-tidy