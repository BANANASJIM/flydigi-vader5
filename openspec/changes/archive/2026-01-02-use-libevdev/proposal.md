# Change: Use libevdev for Input

**Status: Future**

## Why

当前使用原始 uinput ioctl 和 write 系统调用。libevdev 提供：

1. **更简洁的 API** - 无需手动构建 input_event
2. **错误处理** - 内置错误检查
3. **现代 C 库** - 广泛使用于 libinput, wlroots 等
4. **维护性** - 官方 Linux input 子系统库

## What Changes

### Uinput 替换

```cpp
// 当前
input_event ev{};
ev.type = EV_REL;
ev.code = REL_X;
ev.value = dx;
write(fd_, &ev, sizeof(ev));

// libevdev
libevdev_uinput_write_event(uinput_, EV_REL, REL_X, dx);
libevdev_uinput_write_event(uinput_, EV_SYN, SYN_REPORT, 0);
```

### 依赖

```cmake
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
target_link_libraries(vader5d PRIVATE ${LIBEVDEV_LIBRARIES})
```

## Impact

- Affected code: src/uinput.cpp, include/vader5/uinput.hpp
- New dependency: libevdev
- 代码量减少，更易维护