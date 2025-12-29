#pragma once

#include "config.hpp"
#include "hidraw.hpp"
#include "uinput.hpp"

namespace vader5 {

class Gamepad {
public:
    static auto open(const Config& cfg) -> Result<Gamepad>;
    auto poll() -> Result<void>;
    [[nodiscard]] auto fd() const noexcept -> int { return hidraw_.fd(); }

private:
    Gamepad(Hidraw&& hid, Uinput&& uinput)
        : hidraw_(std::move(hid)), uinput_(std::move(uinput)) {}

    Hidraw hidraw_;
    Uinput uinput_;
    GamepadState prev_state_{};
};

} // namespace vader5
