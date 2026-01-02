#include "vader5/config.hpp"
#include "vader5/gamepad.hpp"
#include "vader5/types.hpp"

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <print>

#include <poll.h>

namespace {
std::atomic<bool> g_running{true};

void handle_signal(int /*signum*/) {
    g_running.store(false, std::memory_order_relaxed);
}

constexpr int POLL_TIMEOUT_MS = 100;
} // namespace

auto main() -> int {
    (void)std::signal(SIGINT, handle_signal);
    (void)std::signal(SIGTERM, handle_signal);

    vader5::Config cfg;
    const auto config_path = vader5::Config::default_path();
    if (auto loaded = vader5::Config::load(config_path); loaded) {
        cfg = *loaded;
        std::println("vader5d: Loaded config from {}", config_path);
    }

    std::println("vader5d: Opening Vader 5 Pro (VID:{:04x} PID:{:04x})...", vader5::VENDOR_ID,
                 vader5::PRODUCT_ID);

    auto gamepad = vader5::Gamepad::open(cfg);
    if (!gamepad) {
        std::println(stderr, "vader5d: Failed to open device: {}", gamepad.error().message());
        return 1;
    }

    std::println("vader5d: Virtual gamepad created, running...");

    pollfd pfd{.fd = gamepad->fd(), .events = POLLIN, .revents = 0};

    while (g_running.load(std::memory_order_relaxed)) {
        const int poll_result = poll(&pfd, 1, POLL_TIMEOUT_MS);
        if (poll_result < 0 && errno != EINTR) {
            std::println(stderr, "vader5d: poll error: {}", std::strerror(errno));
            break;
        }

        if (poll_result > 0 && (static_cast<unsigned>(pfd.revents) & POLLIN) != 0) {
            auto result = gamepad->poll();
            if (!result && result.error() != std::errc::resource_unavailable_try_again) {
                std::println(stderr, "vader5d: Read error: {}", result.error().message());
            }
        }
    }

    std::println("vader5d: Shutting down");
    return 0;
}
