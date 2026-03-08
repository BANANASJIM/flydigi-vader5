#include "vader5/config.hpp"
#include "vader5/types.hpp"

#include <cstdlib>
#include <iostream>

using namespace vader5;

#define CHECK(expr)                                                                                \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            std::cerr << "FAIL: " #expr " (" << __FILE__ << ":" << __LINE__ << ")\n";              \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (0)

void test_config_emulate_elite_default() {
    auto cfg = Config::load("config/test-elite-default.toml");
    CHECK(cfg.has_value());
    CHECK(cfg->emulate_elite == true);
    std::cout << "  emulate_elite default (absent): OK\n";
}

void test_config_emulate_elite_true() {
    auto cfg = Config::load("config/test-elite-true.toml");
    CHECK(cfg.has_value());
    CHECK(cfg->emulate_elite == true);
    std::cout << "  emulate_elite = true: OK\n";
}

void test_config_emulate_elite_false() {
    auto cfg = Config::load("config/test-elite-false.toml");
    CHECK(cfg.has_value());
    CHECK(cfg->emulate_elite == false);
    std::cout << "  emulate_elite = false: OK\n";
}

int main() {
    std::cout << "Running uinput elite emulation tests...\n";
    test_config_emulate_elite_default();
    test_config_emulate_elite_true();
    test_config_emulate_elite_false();
    std::cout << "All tests passed!\n";
}
