#include "vader5/debug_options.hpp"

#include <cstdlib>
#include <iostream>

#define CHECK(expr) do { if (!(expr)) { std::cerr << "FAIL: " #expr " (" << __FILE__ << ":" << __LINE__ << ")\n"; std::exit(1); } } while (0)

void test_cli_parse_defaults() {
    const char* args[] = {"vader5-debug"};
    auto opts = vader5::DebugOptions::parse(1, args);
    CHECK(opts.input_iface == -1);
    CHECK(opts.config_iface == 1);
    std::cout << "  cli parse defaults: OK\n";
}

void test_cli_parse_input_override() {
    const char* args[] = {"vader5-debug", "--input", "2"};
    auto opts = vader5::DebugOptions::parse(3, args);
    CHECK(opts.input_iface == 2);
    CHECK(opts.config_iface == 1);
    std::cout << "  cli parse input override: OK\n";
}

void test_cli_parse_config_override() {
    const char* args[] = {"vader5-debug", "--config", "3"};
    auto opts = vader5::DebugOptions::parse(3, args);
    CHECK(opts.input_iface == -1);
    CHECK(opts.config_iface == 3);
    std::cout << "  cli parse config override: OK\n";
}

void test_cli_parse_both_override() {
    const char* args[] = {"vader5-debug", "--input", "2", "--config", "1"};
    auto opts = vader5::DebugOptions::parse(5, args);
    CHECK(opts.input_iface == 2);
    CHECK(opts.config_iface == 1);
    std::cout << "  cli parse both override: OK\n";
}

void test_cli_parse_unknown_ignored() {
    const char* args[] = {"vader5-debug", "--foo", "bar"};
    auto opts = vader5::DebugOptions::parse(3, args);
    CHECK(opts.input_iface == -1);
    CHECK(opts.config_iface == 1);
    std::cout << "  cli parse unknown ignored: OK\n";
}

int main() {
    std::cout << "Running debug interface tests...\n";
    test_cli_parse_defaults();
    test_cli_parse_input_override();
    test_cli_parse_config_override();
    test_cli_parse_both_override();
    test_cli_parse_unknown_ignored();
    std::cout << "All tests passed!\n";
}
