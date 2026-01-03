# Remap System

## ADDED Requirements

### Requirement: 配置文件解析

vader5d SHALL load configuration from ~/.config/vader5/config.toml on startup.

#### Scenario: 加载按键重映射
Given 配置文件包含 [remap] 段
When vader5d 启动
Then 按键映射生效

### Requirement: 按键重映射

All buttons SHALL be remappable to other keycodes, mouse buttons, or disabled.

#### Scenario: 映射到键盘按键
Given config.toml 包含 M1 = "KEY_F13"
When 用户按下 M1
Then 发送 KEY_F13 事件

### Requirement: 陀螺仪鼠标

Gyroscope data SHALL be mappable to mouse movement.

#### Scenario: 陀螺仪控制鼠标
Given gyro.mode = "mouse"
When 用户转动手柄
Then 鼠标光标移动

### Requirement: Mode Shift

The system SHALL support temporary mapping changes when holding a specified button.

#### Scenario: LM 触发鼠标模式
Given mode_shift.LM 配置陀螺仪和按键映射
When 用户按住 LM
Then 陀螺仪控制鼠标，RB/RT 变为鼠标左右键