# Extended Buttons Research

## Architecture (hidraw-only)

Interface 1 in test mode provides ALL functionality:

| Feature | Command/Format |
|---------|---------------|
| Init | `5a a5 01`, `a1`, `02`, `04` |
| Test Mode | `5a a5 11 07 ff 01 ff ff ff 15` |
| Input | `5a a5 ef` extended report (32B) |
| Rumble | `5a a5 12 06 LL RR` |

No libusb needed!

## Extended Report Format (Interface 1, Test Mode)

```
Offset  Size  Field
0-2     3     Magic: 5a a5 ef
3-4     2     Left X (int16)
5-6     2     Left Y (int16)
7-8     2     Right X (int16)
9-10    2     Right Y (int16)
11      1     Buttons (A,B,X,Select + DPad)
12      1     Buttons (Y,Start,LB,RB,L3,R3)
13      1     Ext buttons (C,Z,M1,M2,M3,M4,LM,RM)
14      1     Ext buttons2 (O, Home)
15      1     Left trigger
16      1     Right trigger
17-22   6     Gyro X,Y,Z (int16 each)
23-28   6     Accel X,Y,Z (int16 each)
```

## Button Mapping (Elite Emulation)

uinput 伪装成 Xbox Elite Series 2 (VID 0x045e, PID 0x0b00)，M1-M4 映射匹配 xpad 驱动。

### Byte 13 (ext_buttons)
| Bit | Button | Linux Code | Note |
|-----|--------|------------|------|
| 0 | C | BTN_TRIGGER_HAPPY1 | |
| 1 | Z | BTN_TRIGGER_HAPPY2 | |
| 2 | M1 | BTN_TRIGGER_HAPPY5 | Elite P1 |
| 3 | M3 | BTN_TRIGGER_HAPPY7 | Elite P3 (bit反) |
| 4 | M2 | BTN_TRIGGER_HAPPY6 | Elite P2 (bit反) |
| 5 | M4 | BTN_TRIGGER_HAPPY8 | Elite P4 |
| 6 | LM | BTN_TRIGGER_HAPPY3 | |
| 7 | RM | BTN_TRIGGER_HAPPY4 | |

### Byte 14 (ext_buttons2)
| Bit | Button | Linux Code |
|-----|--------|------------|
| 0 | O | BTN_TRIGGER_HAPPY9 |
| 3 | Home | BTN_MODE (Xbox Guide) |

## Steam Input Compatibility

Based on GitHub issues #12154 and #12462, BTN_TRIGGER_HAPPY buttons have known issues with Steam Input on Linux. However:

1. The buttons ARE exposed via evdev
2. Steam Input CAN see them (with "Extended Buttons" toggle)
3. There are workarounds with udev rules

## Steam Input Config

Steam Input 可以通过编辑 config.vdf 添加 paddle 映射。

**文件位置:** `~/.steam/steam/config/config.vdf`

**格式:**
```
"SDL_GamepadBind" "paddle1:b16,paddle2:b17,paddle3:b18,paddle4:b19,"
```

**注意:**
- Button 编号从 b0 开始 (0-indexed)
- BTN_TRIGGER_HAPPY1 = b16, BTN_TRIGGER_HAPPY2 = b17, etc.
- 需要找到 Vader 5 Pro 虚拟控制器的设备 ID
- 修改后重启 Steam 生效

**参考:** https://steamcommunity.com/groups/bigpicture/discussions/3/7004880943562398178/

## Implementation Status

- [x] Refactor to hidraw-only (Interface 1)
- [x] Send init + test mode on startup
- [x] Parse extended report (5a a5 ef)
- [x] Xbox Elite emulation (VID 0x045e, PID 0x0b00)
- [x] M1-M4 → BTN_TRIGGER_HAPPY5-8 (xpad Elite P1-P4)
- [ ] Steam Input paddle 测试