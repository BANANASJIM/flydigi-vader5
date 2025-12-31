# Add 2.4G Receiver Protocol

## Summary
Implement HID report parsing for Flydigi Vader 5 Pro 2.4G wireless receiver.

## Protocol Details
- Packet length: 20 bytes
- Transfer type: USB Interrupt IN
- Interface type: Vendor Specific (0xFF)
- Fixed field: byte[1] == 0x14

## Byte Layout
| Offset | Length | Description |
|--------|--------|-------------|
| 0 | 1 | Header/Flags (0x00) |
| 1 | 1 | Subtype/Channel (0x14) |
| 2 | 1 | D-pad bit field |
| 3 | 1 | Button bit field (ABXY) |
| 4-5 | 2 | Reserved |
| 6-7 | 2 | Left Stick Y (int16 LE) |
| 8-9 | 2 | Left Stick X (int16 LE) |
| 10-19 | 10 | Reserved / TBD |

## Idle State
```
00 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

## TODO: Complete Mapping
- [ ] Right Stick X/Y offset
- [ ] LT/RT trigger offset
- [ ] LB/RB button bits
- [ ] Extended buttons (M1-M4, C, Z) location
- [ ] D-pad bit values
