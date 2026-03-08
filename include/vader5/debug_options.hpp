#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

namespace vader5 {

struct DebugOptions {
    int input_iface{-1};  // -1 = auto-detect
    int config_iface{1};

    static auto parse(int argc, const char* const* argv) -> DebugOptions {
        DebugOptions opts;
        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            try {
                if (arg == "--input" && i + 1 < argc) {
                    opts.input_iface = std::stoi(argv[++i]);
                } else if (arg == "--config" && i + 1 < argc) {
                    opts.config_iface = std::stoi(argv[++i]);
                }
            } catch (const std::exception&) {
                std::cerr << "Invalid value for " << arg << "\n";
                std::exit(1);
            }
        }
        return opts;
    }
};

} // namespace vader5
