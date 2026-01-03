# Add Remap System

## Summary

添加动态按键重映射系统，支持陀螺仪鼠标模式和 Mode Shift。

## Motivation

1. 用户需要自定义按键映射（如 M1→F13）
2. 陀螺仪可用于模拟鼠标移动
3. Mode Shift 允许按住某键时临时切换功能（如 LM+陀螺仪=鼠标）

## Design

### 架构

```
vader5d (daemon)              vader5-debug --remap (TUI)
     │                              │
     ├── 加载配置                    │
     ├── 处理输入                    │
     ├── Mode Shift 状态机           │
     │                              │
     └────── Unix Socket ───────────┘
            /run/vader5.sock
```

### 配置格式

```toml
# ~/.config/vader5/config.toml

# 按键重映射 (所有按键都可配置)
[remap]
M1 = "KEY_F13"
M2 = "KEY_F14"
M3 = "KEY_F15"
M4 = "KEY_F16"
C = "KEY_TAB"
Z = "KEY_ESC"
O = "disabled"
# 标准按键也可重映射
# A = "KEY_SPACE"
# RB = "mouse_left"

# 陀螺仪配置
[gyro]
mode = "off"              # off / mouse / joystick
sensitivity = 1.5
deadzone = 50
smoothing = 0.8
invert_x = false
invert_y = false
activation = "always"     # always / hold:LM / hold:RM

# 摇杆配置
[stick.left]
deadzone = 128
curve = "linear"          # linear / smooth / aggressive
as_mouse = false          # 摇杆模拟鼠标

[stick.right]
deadzone = 128
curve = "linear"
as_mouse = false

# 扳机配置
[trigger]
left_deadzone = 0
right_deadzone = 0
as_scroll = false         # 扳机模拟滚轮

# Mode Shift: 按住某键时切换模式
[mode_shift.LM]
gyro = "mouse"
RB = "mouse_left"
RT = "mouse_right"
# 可以覆盖任意按键
# A = "mouse_middle"

[mode_shift.RM]
# 另一个 mode shift 配置
gyro = "mouse"
right_stick = "mouse"     # 右摇杆也控制鼠标
```

### 虚拟设备

1. **vader5-gamepad**: 现有手柄设备
2. **vader5-mouse**: 新增虚拟鼠标（REL_X/Y, BTN_LEFT/RIGHT/MIDDLE）

### Mode Shift 状态机

```
Normal Mode ──[LM pressed]──> Mouse Mode
     ^                            │
     └────[LM released]───────────┘
```

Mouse Mode 下：
- 陀螺仪 → REL_X/REL_Y
- 右摇杆 → REL_X/REL_Y（可选）
- RB → BTN_LEFT
- RT → BTN_RIGHT
- 其他按键保持正常

### IPC 协议

Unix socket `/run/vader5.sock`，JSON 消息：

```json
{"cmd": "get_config"}
{"cmd": "set_remap", "button": "M1", "target": "KEY_F13"}
{"cmd": "set_mode_shift", "trigger": "LM", "mappings": {...}}
{"cmd": "reload"}
```

## Scope

### Phase 1 (Current)
- 配置文件解析 (TOML)
- 按键重映射
- 陀螺仪鼠标模式
- Mode Shift

### Phase 2 (Future)
- TUI 配置界面
- IPC 热重载
- 实时参数调节

### Out of Scope
- 宏/按键序列
- 多键组合触发
- 加速度计功能