#include "vader5/config.hpp"
#include "vader5/gamepad.hpp"
#include "vader5/types.hpp"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <print>
#include <thread>

#include <poll.h>

namespace {
std::atomic<bool> g_running{true};

void handle_signal(int /*signum*/) {
    g_running.store(false, std::memory_order_relaxed);
}

constexpr int POLL_TIMEOUT_MS = 100;
constexpr auto RETRY_INTERVAL = std::chrono::seconds(2);
} // namespace

auto main(int argc, char* argv[]) -> int {
    (void)std::signal(SIGINT, handle_signal);
    (void)std::signal(SIGTERM, handle_signal);

    vader5::Config cfg;
    std::string config_path =
        (argc > 1) ? std::string(argv[1]) : vader5::Config::default_path(); // NOLINT
    if (auto loaded = vader5::Config::load(config_path); loaded) {
        cfg = *loaded;
        std::println("vader5d: Loaded config from {}", config_path);
    } else {
        std::println("vader5d: No config at {}, using defaults", config_path);
    }

    std::println("vader5d: Waiting for Vader 5 Pro (VID:{:04x} PID:{:04x})...", vader5::VENDOR_ID,
                 vader5::PRODUCT_ID);

    while (g_running.load(std::memory_order_relaxed)) {
        auto gamepad = vader5::Gamepad::open(cfg);
        if (!gamepad) {
            std::this_thread::sleep_for(RETRY_INTERVAL);
            continue;
        }

        std::println("vader5d: Device connected, running...");
        pollfd pfd{.fd = gamepad->fd(), .events = POLLIN, .revents = 0};

        while (g_running.load(std::memory_order_relaxed)) {
            const int ret = poll(&pfd, 1, POLL_TIMEOUT_MS);
            if (ret < 0 && errno != EINTR) {
                std::println(stderr, "vader5d: poll error: {}", std::strerror(errno));
                break;
            }

            if (ret > 0 && (pfd.revents & POLLIN) != 0) {
                auto result = gamepad->poll();
                if (!result) {
                    auto ec = result.error();
                    if (ec == std::errc::resource_unavailable_try_again) {
                        continue;
                    }
                    if (ec == std::errc::no_such_device || ec == std::errc::io_error) {
                        std::println("vader5d: Device disconnected");
                        break;
                    }
                    std::println(stderr, "vader5d: Read error: {}", ec.message());
                }
            }

            if ((pfd.revents & (POLLHUP | POLLERR)) != 0) {
                std::println("vader5d: Device disconnected");
                break;
            }
        }

        std::println("vader5d: Waiting for reconnection...");
    }

    std::println("vader5d: Shutting down");
    return 0;
}
