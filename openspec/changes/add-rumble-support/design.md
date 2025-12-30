## Context

Vader 5 Pro Interface 0 (Xbox-like):
- EP1 IN 32B: Main input (working)
- EP5 OUT 32B: Output (rumble?)

## Rumble Format Options

### Option 1: Xbox 360 Format (EP5 OUT)
```
Offset  Value
0       0x00 (report type)
1       0x08 (size)
2       0x00
3       left_motor (0-255, low freq)
4       right_motor (0-255, high freq)
5-7     0x00
```

### Option 2: Flydigi Format (EP6 OUT via Interface 1)
```
5a a5 12 06 XX YY 00...
            ^^ ^^
            left right (ff=max, 00=off)
```

## Design

1. Add output URB for EP5 OUT
2. Register FF_RUMBLE effect
3. In ff_play callback, send rumble packet
4. Test Xbox format first, fallback to Flydigi format if needed

## Implementation

```c
struct flydigi_data {
    // ... existing fields
    struct urb *irq_out;
    unsigned char *odata;
    dma_addr_t odata_dma;
    struct work_struct rumble_work;
    u8 left_motor;
    u8 right_motor;
};
```