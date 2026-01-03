## Tasks

### Phase 1: 配置系统

- [x] 添加 toml 解析库依赖
- [x] 定义 Config 结构体
- [x] 解析 ~/.config/vader5/config.toml
- [x] 按键名→键码映射表

### Phase 2: 按键重映射

- [x] 所有按键可重映射
- [x] 支持目标: 键码 / 鼠标按钮 / disabled
- [ ] 应用映射到 uinput emit

### Phase 3: 虚拟鼠标

- [x] 创建 vader5-mouse uinput 设备
- [x] 支持 REL_X/Y, BTN_LEFT/RIGHT/MIDDLE, REL_WHEEL

### Phase 4: 陀螺仪鼠标

- [x] 陀螺仪→鼠标移动转换
- [x] 配置: sensitivity, deadzone, smoothing, invert

### Phase 5: Mode Shift

- [x] Mode Shift 状态机
- [x] 按住触发键切换模式
- [x] Mode Shift 下的陀螺仪/摇杆映射
- [x] Mode Shift 下的按键映射 (RB→mouse_left)

### Phase 6: 摇杆/扳机配置

- [x] 摇杆死区
- [x] 摇杆→鼠标模式
- [ ] 曲线配置
- [ ] 扳机死区、滚轮模式

### Future: TUI

- [ ] Unix socket IPC
- [ ] vader5-debug --config 模式
- [ ] 实时配置修改
- [ ] 热重载