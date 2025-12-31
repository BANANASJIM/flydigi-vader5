# Add libusb Support

## Summary
Use libusb to directly read USB interrupt transfers, bypassing hidraw.

## Rationale
- Interface 0 has no driver bound, hidraw not available
- libusb allows direct USB access without kernel driver
- More control over device communication

## Implementation
1. Add libusb-1.0 dependency
2. Create UsbDevice class to replace Hidraw
3. Claim interface 0, read interrupt endpoint
4. Parse 20-byte vendor packets

## API
```cpp
class UsbDevice {
    static auto open(uint16_t vid, uint16_t pid) -> Result<UsbDevice>;
    auto read(std::span<uint8_t> buf, int timeout_ms) -> Result<size_t>;
};
```