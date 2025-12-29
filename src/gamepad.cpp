#include "vader5/gamepad.hpp"

#include <array>

namespace vader5 {

namespace {
constexpr size_t READ_BUFFER_SIZE = 64;
} // namespace

auto Gamepad::open(const Config& cfg) -> Result<Gamepad> {
    constexpr int GAMEPAD_INTERFACE = 2;
    auto hid = Hidraw::open(VENDOR_ID, PRODUCT_ID, GAMEPAD_INTERFACE);
    if (!hid) {
        return std::unexpected(hid.error());
    }

    auto uinput = Uinput::create(cfg.ext_mappings);
    if (!uinput) {
        return std::unexpected(uinput.error());
    }

    return Gamepad(std::move(*hid), std::move(*uinput));
}

auto Gamepad::poll() -> Result<void> {
    std::array<uint8_t, READ_BUFFER_SIZE> buf{};
    const auto bytes_read = hidraw_.read(buf);
    if (!bytes_read) {
        return std::unexpected(bytes_read.error());
    }

    const auto state = Hidraw::parse_report({buf.data(), *bytes_read});
    if (!state) {
        return {};
    }

    auto result = uinput_.emit(*state, prev_state_);
    prev_state_ = *state;
    return result;
}

} // namespace vader5
