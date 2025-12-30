#include "vader5/transport.hpp"

namespace vader5 {

UsbTransport::UsbTransport(libusb_context* ctx, libusb_device_handle* handle,
                           uint8_t iface, uint8_t endpoint)
    : ctx_(ctx), handle_(handle), interface_(iface), endpoint_(endpoint), claimed_(true) {}

auto UsbTransport::open(uint16_t vid, uint16_t pid, uint8_t iface, uint8_t endpoint)
    -> Result<UsbTransport> {
    libusb_context* ctx = nullptr;
    if (libusb_init(&ctx) != 0) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }

    auto* handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
    if (handle == nullptr) {
        libusb_exit(ctx);
        return std::unexpected(std::make_error_code(std::errc::no_such_device));
    }

    if (libusb_kernel_driver_active(handle, iface) == 1) {
        libusb_detach_kernel_driver(handle, iface);
    }

    if (libusb_claim_interface(handle, iface) != 0) {
        libusb_close(handle);
        libusb_exit(ctx);
        return std::unexpected(std::make_error_code(std::errc::device_or_resource_busy));
    }

    return UsbTransport(ctx, handle, iface, endpoint);
}

UsbTransport::~UsbTransport() {
    if (handle_ != nullptr) {
        if (claimed_) {
            libusb_release_interface(handle_, interface_);
        }
        libusb_close(handle_);
    }
    if (ctx_ != nullptr) {
        libusb_exit(ctx_);
    }
}

UsbTransport::UsbTransport(UsbTransport&& other) noexcept
    : ctx_(other.ctx_), handle_(other.handle_), interface_(other.interface_),
      endpoint_(other.endpoint_), claimed_(other.claimed_) {
    other.ctx_ = nullptr;
    other.handle_ = nullptr;
    other.claimed_ = false;
}

auto UsbTransport::operator=(UsbTransport&& other) noexcept -> UsbTransport& {
    if (this != &other) {
        this->~UsbTransport();
        ctx_ = other.ctx_;
        handle_ = other.handle_;
        interface_ = other.interface_;
        endpoint_ = other.endpoint_;
        claimed_ = other.claimed_;
        other.ctx_ = nullptr;
        other.handle_ = nullptr;
        other.claimed_ = false;
    }
    return *this;
}

auto UsbTransport::read(std::span<uint8_t> buf, int timeout_ms) -> Result<size_t> {
    int transferred = 0;
    const int ret = libusb_interrupt_transfer(handle_, endpoint_ | LIBUSB_ENDPOINT_IN,
                                        buf.data(), static_cast<int>(buf.size()),
                                        &transferred, timeout_ms);
    if (ret == LIBUSB_ERROR_TIMEOUT) {
        return 0;
    }
    if (ret != 0) {
        return std::unexpected(std::make_error_code(std::errc::io_error));
    }
    return static_cast<size_t>(transferred);
}

auto UsbTransport::is_open() const noexcept -> bool {
    return handle_ != nullptr;
}

} // namespace vader5