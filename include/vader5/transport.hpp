#pragma once

#include "types.hpp"

#include <libusb-1.0/libusb.h>

#include <memory>
#include <span>

namespace vader5 {

class UsbTransport {
public:
    static auto open(uint16_t vid, uint16_t pid, uint8_t iface, uint8_t endpoint)
        -> Result<UsbTransport>;
    ~UsbTransport();

    UsbTransport(UsbTransport&& other) noexcept;
    auto operator=(UsbTransport&& other) noexcept -> UsbTransport&;
    UsbTransport(const UsbTransport&) = delete;
    auto operator=(const UsbTransport&) -> UsbTransport& = delete;

    [[nodiscard]] auto read(std::span<uint8_t> buf, int timeout_ms = 1000) -> Result<size_t>;
    [[nodiscard]] auto write(std::span<const uint8_t> buf, uint8_t ep_out, int timeout_ms = 1000)
        -> Result<size_t>;
    [[nodiscard]] auto is_open() const noexcept -> bool;

private:
    UsbTransport(libusb_context* ctx, libusb_device_handle* handle, uint8_t iface, uint8_t endpoint);

    libusb_context* ctx_{nullptr};
    libusb_device_handle* handle_{nullptr};
    uint8_t interface_{0};
    uint8_t endpoint_{0};
    bool claimed_{false};
};

} // namespace vader5