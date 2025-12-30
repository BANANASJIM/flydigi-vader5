# Change: TUI Debug/Config Tool

## Why

- `vader5-debug` 需要改为 TUI 界面
- 支持实时查看输入状态
- 支持配置按键映射、振动、LED

## What Changes

1. 使用 libusb 读取 Interface 0 (主输入)
2. 使用 hidraw 读写 Interface 1 (配置命令)
3. 添加 TUI 界面 (ncurses/ftxui)
4. 支持配置功能：
   - 按键映射 (remap)
   - 振动测试
   - LED 配置
   - 测试模式开关

## Impact

- 重构: src/debug.cpp → src/tui.cpp
- 新增: 配置命令发送功能