#include "vader5/config.hpp"
#include "vader5/gamepad.hpp"
#include "vader5/types.hpp"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <format>
#include <iostream>
#include <span>
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

    std::string config_path = vader5::Config::default_path();
    const std::span args(argv, static_cast<size_t>(argc)); // NOLINT
    for (size_t i = 1; i < args.size(); ++i) {
        if ((std::strcmp(args[i], "-c") == 0 || std::strcmp(args[i], "--config") == 0) &&
            i + 1 < args.size()) {
            config_path = args[++i];
        }
    }

    vader5::Config cfg;
    if (auto loaded = vader5::Config::load(config_path); loaded) {
        cfg = *loaded;
        std::cout << std::format("vader5d: Loaded config from {}\n", config_path);
    } else {
        std::cout << std::format("vader5d: No config at {}, using defaults\n", config_path);
    }

    std::cout << std::format("vader5d: Waiting for Vader 5 Pro (VID:{:04x} PID:{:04x})...\n",
                             vader5::VENDOR_ID, vader5::PRODUCT_ID);

    while (g_running.load(std::memory_order_relaxed)) {
        auto gamepad = vader5::Gamepad::open(cfg);
        if (!gamepad) {
            std::this_thread::sleep_for(RETRY_INTERVAL);
            continue;
        }

        std::cout << "vader5d: Device connected, running...\n";
        pollfd pfd{.fd = gamepad->fd(), .events = POLLIN, .revents = 0};

        while (g_running.load(std::memory_order_relaxed)) {
            const int ret = poll(&pfd, 1, POLL_TIMEOUT_MS);
            if (ret < 0 && errno != EINTR) {
                std::cerr << std::format("vader5d: poll error: {}\n", std::strerror(errno));
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
                        std::cout << "vader5d: Device disconnected\n";
                        break;
                    }
                    std::cerr << std::format("vader5d: Read error: {}\n", ec.message());
                }
            }

            if ((pfd.revents & (POLLHUP | POLLERR)) != 0) {
                std::cout << "vader5d: Device disconnected\n";
                break;
            }
        }

        std::cout << "vader5d: Waiting for reconnection...\n";
    }

    std::cout << "vader5d: Shutting down\n";
    return 0;
}
