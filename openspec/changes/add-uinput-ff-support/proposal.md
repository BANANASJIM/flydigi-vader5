# Change: Add Uinput Force Feedback Support

## Why

The current `send_rumble()` function exists but is never called. Games using SDL/Steam Input send force feedback events to the virtual gamepad via the Linux input subsystem. Without receiving these events, rumble requests from games are ignored.

## What Changes

- Enable FF_RUMBLE capability in uinput virtual gamepad
- Open uinput with read permission to receive FF events
- Poll uinput fd for incoming FF upload/play events
- Forward rumble requests to hardware via existing `send_rumble()`

## Impact

- Affected code: `uinput.cpp`, `gamepad.cpp`, `main.cpp`
- Modified spec: driver (adds FF receive capability)
- Games can now trigger controller vibration
