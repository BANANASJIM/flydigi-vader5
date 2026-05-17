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
#include <memory>
#include <optional>
#include <span>
#include <thread>

#include <poll.h>

#ifdef VADER5_USE_HIDAPI
#include <sys/timerfd.h>
#include <unistd.h>
#endif

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

#ifdef VADER5_USE_HIDAPI
    // Timerfd-based polling loop: hidapi does not expose a pollable fd,
    // so we use a 1ms timerfd to wake the loop periodically and call
    // hid_read_timeout() which internally does its own short timeout.
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd < 0) {
        std::cerr << "vader5d: Failed to create timerfd: " << std::strerror(errno) << "\n";
        return 1;
    }
    itimerspec timer_spec{};
    timer_spec.it_value.tv_nsec = 1000000;     // 1ms initial
    timer_spec.it_interval.tv_nsec = 1000000;  // 1ms periodic
    timerfd_settime(timer_fd, 0, &timer_spec, nullptr);
#endif

    // Create gamepad once to keep uinput device stable across reconnections
    std::unique_ptr<vader5::Gamepad> gamepad;
    while (g_running.load(std::memory_order_relaxed) && !gamepad) {
        auto gp = vader5::Gamepad::open(cfg, device_name);
        if (gp) {
            gamepad = std::make_unique<vader5::Gamepad>(std::move(*gp));
            std::cout << "vader5d: Device connected, running..." << std::endl;
        } else {
            std::this_thread::sleep_for(RETRY_INTERVAL);
        }
    }

    while (g_running.load(std::memory_order_relaxed) && gamepad) {
#ifdef VADER5_USE_HIDAPI
        // ---- hidapi backend ----
        int ff_fd = gamepad->ff_fd();

        sigset_t old_mask;
        sigprocmask(SIG_BLOCK, &block_mask, &old_mask);

        while (g_running.load(std::memory_order_relaxed)) {
            auto result = gamepad->poll();
            if (!result) {
                auto ec = result.error();
                if (ec == std::errc::resource_unavailable_try_again) {
                    // No data available, normal
                } else if (ec == std::errc::no_such_device || ec == std::errc::io_error) {
                    std::cout << "vader5d: Device disconnected, reconnecting...\n";
                    break;
                } else {
                    std::cerr << "vader5d: Read error: " << ec.message() << "\n";
                }
            }

            gamepad->poll_ff();

            std::array<pollfd, 2> pfds{};
            pfds[0].fd = timer_fd;
            pfds[0].events = POLLIN;
            int nfds = 1;

            if (ff_fd >= 0) {
                pfds[1].fd = ff_fd;
                pfds[1].events = POLLIN;
                nfds = 2;
            }

            const int ret = ppoll(pfds.data(), nfds, nullptr, &empty_mask);
            if (ret < 0) {
                const int err = errno;
                if (err == EINTR) {
                    continue;
                }
                std::cerr << "vader5d: poll error: " << std::strerror(err) << "\n";
                break;
            }

            if (pfds[0].revents & POLLIN) {
                uint64_t expirations;
                ::read(timer_fd, &expirations, sizeof(expirations));
            }
        }
        sigprocmask(SIG_SETMASK, &old_mask, nullptr);

        // Reconnect hidapi while keeping uinput alive
        if (g_running.load(std::memory_order_relaxed)) {
            std::cout << "vader5d: Waiting for device to reconnect...\n";
            while (g_running.load(std::memory_order_relaxed)) {
                auto rc = gamepad->reconnect_hid();
                if (rc) {
                    std::cout << "vader5d: Device reconnected, running...\n";
                    break;
                }
                std::this_thread::sleep_for(RETRY_INTERVAL);
            }
        }
#else
        // ---- hidraw backend (original) ----
        std::array<pollfd, 2> pfds{{
            {.fd = gamepad->fd(), .events = POLLIN, .revents = 0},
            {.fd = gamepad->ff_fd(), .events = POLLIN, .revents = 0},
        }};

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

        // hidraw backend: reconnect gamepad (destroys and recreates uinput)
        if (g_running.load(std::memory_order_relaxed)) {
            std::cout << "vader5d: Waiting for device to reconnect...\n";
            // Gamepad destructor will destroy uinput; loop restarts from top
            gamepad.reset();
            while (g_running.load(std::memory_order_relaxed) && !gamepad) {
                auto gp = vader5::Gamepad::open(cfg, device_name);
                if (gp) {
                    gamepad = std::make_unique<vader5::Gamepad>(std::move(*gp));
                    std::cout << "vader5d: Device reconnected, running...\n";
                } else {
                    std::this_thread::sleep_for(RETRY_INTERVAL);
                }
            }
        }
#endif
    }

#ifdef VADER5_USE_HIDAPI
    ::close(timer_fd);
#endif
    std::cout << "vader5d: Shutting down\n";
    return 0;
}
