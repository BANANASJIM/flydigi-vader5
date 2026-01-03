# Vader 5 Pro USB Protocol

## Device Info

- VID: 0x37d7
- PID: 0x2401
- Connection: 2.4G USB dongle

## USB Interface Layout

| Interface | Class | Endpoints | Purpose |
|-----------|-------|-----------|---------|
| 0 | Vendor (0xff/0x5d Xbox) | EP1 IN 20B, EP5 OUT 8B | Main input + Rumble |
| 1 | HID | EP2 IN 32B, EP6 OUT 32B | Config commands + Extended input |
| 2 | HID | EP2 IN 32B | (Unused) |
| 3 | HID | EP4 IN 64B | Keyboard mode |

## Input Reports

### Interface 0 / EP1 IN: Standard Input (20 bytes)

Standard Xbox-like input report.

```
Offset  Size  Description
------  ----  -----------
0       1     Report ID (0x00)
1       1     Subtype (0x14 = 2.4G)
2       1     misc: D-Pad + Start/Select/L3/R3
3       1     buttons: LB/RB/Home + ABXY
4       1     Left trigger LT (0-255)
5       1     Right trigger RT (0-255)
6-7     2     Left stick X (int16 LE)
8-9     2     Left stick Y (int16 LE)
10-11   2     Right stick X (int16 LE)
12-13   2     Right stick Y (int16 LE)
14-19   6     Reserved
```

### Interface 1 / EP2 IN: Extended Input + IMU (32 bytes)

Requires test mode. Magic: `5a a5 ef`

```
Offset  Size  Description
------  ----  -----------
0-2     3     Magic (5a a5 ef)
3-4     2     Left stick X (int16 LE)
5-6     2     Left stick Y (int16 LE)
7-8     2     Right stick X (int16 LE)
9-10    2     Right stick Y (int16 LE)
11      1     Buttons 1 (DPad + A/B/Select/X)
12      1     Buttons 2 (Y/Start/LB/RB/L3/R3)
13      1     Ext buttons (C/Z/M1-M4/LM/RM)
14      1     Ext buttons 2 (O/Home)
15      1     Left trigger (0-255)
16      1     Right trigger (0-255)
17-18   2     Gyro X (int16 LE)
19-20   2     Gyro Y (int16 LE)
21-22   2     Gyro Z (int16 LE)
23-24   2     Accel X (int16 LE, 4096 = 1g)
25-26   2     Accel Y (int16 LE)
27-28   2     Accel Z (int16 LE)
29-31   3     Reserved
```

**Byte 11 - Buttons 1:**

| Bit | Button |
|-----|--------|
| 0 | D-Pad Up |
| 1 | D-Pad Down |
| 2 | D-Pad Left |
| 3 | D-Pad Right |
| 4 | A |
| 5 | B |
| 6 | Select |
| 7 | X |

**Byte 12 - Buttons 2:**

| Bit | Button |
|-----|--------|
| 0 | Y |
| 1 | Start |
| 2 | LB |
| 3 | RB |
| 6 | L3 |
| 7 | R3 |

**Byte 13 - Extended Buttons:**

| Bit | Button |
|-----|--------|
| 0 | C |
| 1 | Z |
| 2 | M1 |
| 3 | M2 |
| 4 | M3 |
| 5 | M4 |
| 6 | LM |
| 7 | RM |

**Byte 14 - Extended Buttons 2:**

| Bit | Button |
|-----|--------|
| 0 | O |
| 3 | Home |

## Rumble (Interface 0 / EP5 OUT)

Xbox 360 format, 8 bytes:

```
Offset  Value
------  -----
0       0x00
1       0x08
2       0x00
3       left_motor (0-255)
4       right_motor (0-255)
5-7     0x00
```

## Flydigi Commands (Interface 1 / EP6 OUT)

32-byte packets, magic: `5a a5`

### Initialization Sequence

Required after device connection:

```
5a a5 01 02 03  (device info)
5a a5 a1 02 a3  (MAC/serial)
5a a5 02 02 04  (config read)
5a a5 04 02 06  (config data)
```

### Test Mode (11 07)

Enable extended input + IMU:

```
5a a5 11 07 ff 01 ff ff ff 15 00...  (enable)
5a a5 11 07 ff 00 ff ff ff 14 00...  (disable)
```

### Rumble Test (12 06)

```
5a a5 12 06 LL RR 00...
            ^^ ^^
            left/right motor (0x00-0xff)
```

### Profile Switch (a2 03)

```
5a a5 a2 03 XX YY 00...
            ^^
            profile (0-3)
```

## References

- [vader3](https://github.com/ahungry/vader3) - Initial protocol research
- [flydigictl](https://github.com/pipe01/flydigictl) - Bluetooth protocol reference