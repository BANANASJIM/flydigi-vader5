#pragma once

#include "types.hpp"

namespace vader5 {

class Mouse {
  public:
    static auto create(const char* name = "Vader 5 Pro Virtual Mouse") -> Result<Mouse>;
    ~Mouse();

    Mouse(Mouse&& other) noexcept;
    auto operator=(Mouse&& other) noexcept -> Mouse&;
    Mouse(const Mouse&) = delete;
    auto operator=(const Mouse&) -> Mouse& = delete;

    void move(int dx, int dy) const;
    void button(int code, bool pressed) const;
    void scroll(int delta) const;
    void sync() const;

  private:
    explicit Mouse(int fd) : fd_(fd) {}
    int fd_{-1};
};

} // namespace vader5