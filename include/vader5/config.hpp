#pragma once

#include "types.hpp"

#include <array>
#include <optional>
#include <string>

namespace vader5 {

struct Config {
    std::array<std::optional<int>, 8> ext_mappings{};
    int deadzone{4000};

    static auto load(const std::string& path) -> Result<Config>;
    static auto default_path() -> std::string;
};

auto keycode_from_name(std::string_view name) -> std::optional<int>;

} // namespace vader5
