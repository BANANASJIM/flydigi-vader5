# LED/Lighting Control

## Why

Vader 5 Pro has RGB lighting. Protocol documented in research/protocol.md.

## What Changes

- Add LED config to config.toml (base + per-layer)
- Implement LED control via HID command `a9 17`
- Initial modes: static, off, breathing

## Config Example

```toml
[led]
mode = "static"       # static / off / breathing
brightness = 80
color = "#00ff00"

[layer.aim]
trigger = "LM"
led = { mode = "breathing", color = "#ff0000" }

[layer.mouse]
trigger = "RM"
led = { mode = "static", color = "#0000ff" }
```

Layer LED changes when layer activates, reverts when deactivated.

## Dynamic Control Interface

Unix socket `/run/vader5.sock` for external LED control:

```json
{"cmd": "led", "mode": "static", "color": "#ff0000", "brightness": 100}
```

Use cases:
- Game integration (health bar → LED color)
- Music visualization
- Notifications