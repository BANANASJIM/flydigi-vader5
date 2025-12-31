## Context

Vader 5 Pro 手柄协议已逆向完成，需要实现配置工具。
工具需要：
- 实时读取手柄输入
- 发送配置命令到手柄
- 提供交互式 TUI 界面

## Goals / Non-Goals

**Goals:**
- 轻量级，快速启动
- 跨平台 (Linux 优先)
- 现代 C++17 代码风格
- 无需 root 权限 (通过 udev 规则)

**Non-Goals:**
- 不实现宏/连招编辑
- 不实现固件升级
- 不支持蓝牙模式 (仅 2.4G USB)

## Decisions

### TUI 框架: FTXUI

**选择理由:**
- 纯 C++17，无系统依赖
- CMake FetchContent 集成简单
- 现代组件化设计
- 支持动画和异步更新
- MIT 协议

**备选方案:**
- termbox2: 更轻量但功能有限
- ncurses: 广泛支持但 API 老旧
- 纯 ANSI: 最轻量但开发成本高

### 架构设计

```
┌─────────────────────────────────────────────────────┐
│                    TUI Layer (FTXUI)                │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐   │
│  │ Input   │ │ Mapping │ │ Vibrate │ │  LED    │   │
│  │ Display │ │ Editor  │ │ Config  │ │ Config  │   │
│  └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘   │
├───────┼───────────┼───────────┼───────────┼─────────┤
│       └───────────┴───────────┴───────────┘         │
│                    Config Manager                    │
├─────────────────────────────────────────────────────┤
│                   Protocol Layer                     │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐   │
│  │ Command │ │ Report  │ │ Profile │ │   LED   │   │
│  │ Builder │ │ Parser  │ │ Manager │ │ Manager │   │
│  └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘   │
├───────┴───────────┴───────────┴───────────┴─────────┤
│                    HID Layer                         │
│                  (hidraw read/write)                 │
└─────────────────────────────────────────────────────┘
```

### 命令构建

```cpp
namespace vader5::cmd {

// 命令魔数
constexpr uint8_t MAGIC[] = {0x5a, 0xa5};

// 命令类型
enum class Type : uint16_t {
    TestMode    = 0x1107,
    Vibrate     = 0x1206,
    Profile     = 0xa203,
    MapSingle   = 0xa406,
    MapBatch    = 0xa517,
    Save        = 0xa604,
    LedSingle   = 0xa806,
    LedBatch    = 0xa917,
};

// 构建命令
auto build_profile_switch(uint8_t profile) -> std::array<uint8_t, 32>;
auto build_button_map(uint8_t src, uint8_t dst) -> std::array<uint8_t, 32>;
auto build_vibration(uint8_t left, uint8_t right) -> std::array<uint8_t, 32>;
auto build_led_mode(LedMode mode, uint8_t brightness) -> std::array<uint8_t, 32>;

} // namespace vader5::cmd
```

### 按键编码

```cpp
namespace vader5::btn {

// 目标按键编码
enum class Target : uint8_t {
    DpadUp    = 0x00,
    DpadRight = 0x01,
    DpadDown  = 0x02,
    DpadLeft  = 0x03,
    A         = 0x04,
    B         = 0x05,
    X         = 0x06,
    Y         = 0x07,
    Select    = 0x08,
    Start     = 0x09,
    LB        = 0x0a,
    RB        = 0x0b,
    LT        = 0x0c,
    RT        = 0x0d,
    L3        = 0x0e,
    R3        = 0x0f,
    None      = 0xff,
};

// 源按键索引 (扩展按键)
enum class Source : uint8_t {
    C  = 0,
    Z  = 1,
    M1 = 2,
    M2 = 3,
    M3 = 4,
    M4 = 5,
    LM = 6,
    RM = 7,
    O  = 8,
};

} // namespace vader5::btn
```

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| FTXUI 增加编译时间 | 使用 precompiled headers |
| hidraw 权限问题 | 提供 udev 规则安装脚本 |
| 协议不完整 | 优雅降级，未知命令跳过 |

## File Structure

```
src/
├── cli/
│   ├── main.cpp              # 入口
│   ├── app.hpp               # FTXUI 应用
│   ├── components/
│   │   ├── input_display.hpp # 输入显示
│   │   ├── mapping_editor.hpp# 映射编辑
│   │   ├── vibration.hpp     # 振动配置
│   │   └── led_config.hpp    # LED 配置
│   └── protocol/
│       ├── commands.hpp      # 命令构建
│       ├── reports.hpp       # 报告解析
│       └── constants.hpp     # 常量定义
└── vader5/
    └── hidraw.hpp            # HID 通信 (已存在)
```

## Open Questions

- [ ] 是否需要支持多手柄同时连接?
- [ ] 是否需要配置文件导入/导出?
- [ ] LED 颜色选择器如何设计?
