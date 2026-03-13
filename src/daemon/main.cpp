#include "vader5/config.hpp"
#include "vader5/gamepad.hpp"
#include "vader5/types.hpp"

#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <span>
#include <thread>

#include <poll.h>

namespace {
std::atomic<bool> g_running{true};

void handle_signal(int /*signum*/) {
    g_running.store(false, std::memory_order_relaxed);
}

constexpr auto RETRY_INTERVAL = std::chrono::seconds(2);
} // namespace

auto main(int argc, char* argv[]) -> int {
    std::string config_path = vader5::Config::default_path();
    std::string device_name;
    const std::span args(argv, static_cast<size_t>(argc)); // NOLINT
    for (size_t i = 1; i < args.size(); ++i) {
        if ((std::strcmp(args[i], "-c") == 0 || std::strcmp(args[i], "--config") == 0) &&
            i + 1 < args.size()) {
            config_path = args[++i];
        } else if ((std::strcmp(args[i], "-d") == 0 || std::strcmp(args[i], "--device") == 0) &&
                   i + 1 < args.size()) {
            device_name = args[++i];
        }
    }

    vader5::Config cfg;
    if (auto loaded = vader5::Config::load(config_path); loaded) {
        cfg = *loaded;
        std::cout << "vader5d: Loaded config from " << config_path << "\n";
    } else {
        std::cout << "vader5d: No config at " << config_path << ", using defaults\n";
    }

    std::cout << "vader5d: Waiting for Vader 5 Pro (VID:"
              << std::hex << std::setfill('0') << std::setw(4) << vader5::VENDOR_ID
              << " PID:" << std::setw(4) << vader5::PRODUCT_ID << std::dec << ")...\n";

    struct sigaction sa {};
    sa.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);

    sigset_t empty_mask;
    sigemptyset(&empty_mask);

    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGTERM);
    sigaddset(&block_mask, SIGINT);

    while (g_running.load(std::memory_order_relaxed)) {
        auto gamepad = vader5::Gamepad::open(cfg, device_name);
        if (!gamepad) {
            std::this_thread::sleep_for(RETRY_INTERVAL);
            continue;
        }

        std::cout << "vader5d: Device connected, running...\n";
        std::array<pollfd, 2> pfds{{
            {.fd = gamepad->fd(), .events = POLLIN, .revents = 0},
            {.fd = gamepad->ff_fd(), .events = POLLIN, .revents = 0},
        }};

        // Block signals except when we're polling, which allows an indefinite poll without a race
        // where we may miss a signal
        sigset_t old_mask;
        sigprocmask(SIG_BLOCK, &block_mask, &old_mask);

        while (g_running.load(std::memory_order_relaxed)) {
            const int ret = ppoll(pfds.data(), pfds.size(), nullptr, &empty_mask);
            if (ret < 0) {
                const int err = errno;
                if (err == EINTR) {
                    continue;
                }
                std::cerr << "vader5d: poll error: " << std::strerror(err) << "\n";
                break;
            }

            if (ret > 0 && (pfds[0].revents & POLLIN) != 0) {
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
                    std::cerr << "vader5d: Read error: " << ec.message() << "\n";
                }
            }

            if (ret > 0 && (pfds[1].revents & POLLIN) != 0) {
                gamepad->poll_ff();
            }

            if ((pfds[0].revents & (POLLHUP | POLLERR)) != 0) {
                std::cout << "vader5d: Device disconnected\n";
                break;
            }
        }
        sigprocmask(SIG_SETMASK, &old_mask, nullptr);

        std::cout << "vader5d: Waiting for reconnection...\n";
    }

    std::cout << "vader5d: Shutting down\n";
    return 0;
}
