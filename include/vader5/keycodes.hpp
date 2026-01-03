#pragma once

#include <linux/input-event-codes.h>

#include <optional>
#include <string_view>

namespace vader5 {

auto keycode_from_name(std::string_view name) -> std::optional<int>;

} // namespace vader5