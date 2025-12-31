# Change: Add Interactive Config CLI

## Why

基于逆向工程获得的协议，需要一个轻量级的命令行工具来配置 Vader 5 Pro 手柄。
现有 add-tui 方案使用 termbox2，本方案采用更现代的 FTXUI 库，提供更好的跨平台支持和更丰富的 UI 组件。

## What Changes

- 新增 `vader5-config` 命令行配置工具
- 支持实时显示手柄输入状态
- 支持交互式修改按键映射
- 支持配置档切换 (4 档)
- 支持振动强度设置
- 支持 LED 灯效配置
- 使用 FTXUI 作为 TUI 框架 (单头文件，MIT 协议)

## Dependencies

- **FTXUI** - Modern C++ TUI library
  - CMake FetchContent 自动获取
  - 无系统依赖 (纯 C++17)
  - 支持 Linux/macOS/Windows

## Features

### 1. 实时输入显示
- 按键状态可视化
- 摇杆位置图形显示
- 扳机压力条
- 扩展按键 (M1-M4, C, Z, LM, RM, O)

### 2. 按键映射编辑
- 选择源按键 (扩展按键)
- 选择目标按键 (标准按键)
- 实时预览映射效果
- 保存到板载存储

### 3. 配置档管理
- 切换 4 个配置档
- 显示当前配置档
- 每个档位独立配置

### 4. 振动设置
- 左右马达强度 (0-100%)
- 实时测试振动

### 5. LED 配置
- 模式选择 (常亮/呼吸/渐变/循环/流光/关闭)
- 亮度调节 (0-100)
- 循环速度调节
- 颜色选择

## UI Layout

```
╭─ Vader 5 Pro Config ─────────────────────────────────────────╮
│                                                               │
│  ┌─ Input ────────────────────────────────────────────────┐  │
│  │  [LT ████░░░░░░ 60%]              [RT ░░░░░░░░░░  0%]  │  │
│  │                                                         │  │
│  │     [LB]                              [RB]             │  │
│  │                                                         │  │
│  │   ┌─────┐    [Y]    [M1][M2]    ┌─────┐               │  │
│  │   │  ●  │  [X] [B]  [M3][M4]    │     │               │  │
│  │   └─────┘    [A]    [LM][RM]    └─────┘               │  │
│  │    L-Stick           [C][Z][O]    R-Stick              │  │
│  │                                                         │  │
│  │   [↑]      [SEL] [HOME] [START]                        │  │
│  │  [←][→]                                                 │  │
│  │   [↓]                                                   │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                               │
│  Profile: [1] [2] [3] [4]    Vibration: L[██████] R[████░░]  │
│                                                               │
│  [Tab] Mapping  [V] Vibration  [L] LED  [P] Profile  [Q] Quit│
╰───────────────────────────────────────────────────────────────╯
```

## Impact

- Affected specs: 新增 `config-cli` capability
- Affected code:
  - 新增 `src/cli/` 目录
  - 新增 `vader5-config` 可执行文件
  - 复用现有 `hidraw.hpp` 通信层
