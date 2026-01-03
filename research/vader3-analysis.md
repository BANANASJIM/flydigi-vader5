# vader3 驱动分析

来源: https://github.com/ahungry/vader3

## 概述
- 蓝牙 HID 驱动 (不支持 USB)
- VID: 0xD7D7, PID: 0x0041 (蓝牙)
- GPL v3 许可证

## 驱动结构

```c
static struct hid_driver vader3_driver = {
    .name = "vader3",
    .id_table = vader3_devices,
    .input_mapping = vader3_input_mapping,      // 返回 0，使用默认映射
    .input_configured = vader3_input_configured, // 初始化输入设备
    .raw_event = vader3_raw_event,              // 预处理原始数据
    .event = vader3_event,                      // 事件重映射
};
```

## 按键映射

| 原始事件 | 映射到 |
|---------|--------|
| BTN_0 | BTN_TRIGGER_HAPPY1 (C 键) |
| KEY_ENTER | BTN_TRIGGER_HAPPY2 (Z 键) |
| KEY_CHANNELUP | BTN_TRIGGER_HAPPY3 (M1) |
| KEY_TV | BTN_TRIGGER_HAPPY4 (M2) |
| KEY_CHANNELDOWN | BTN_TRIGGER_HAPPY5 (M3) |
| KEY_PROGRAM | BTN_TRIGGER_HAPPY6 (M4) |
| KEY_CAMERA | BTN_TRIGGER_HAPPY7 (Circle) |
| KEY_RED | BTN_TRIGGER_HAPPY8 (Home) |

## D-Pad 处理

蓝牙模式下 D-Pad 值全部通过 ABS_HAT0X 上报:
- 1=上, 3=右, 5=下, 7=左
- 2=右上, 4=右下, 6=左下, 8=左上
- 0=释放

驱动在 event 函数中转换为标准 ABS_HAT0X/ABS_HAT0Y。

## raw_event 修复

```c
// 修复 M4 按键 (bit 4 -> bit 64)
if (data[11] & 4) {
    data[11] |= 64;
    data[11] &= ~4;
}
```

## 与 Vader 5 Pro 的差异

| 特性 | Vader 3 (BT) | Vader 5 Pro (2.4G USB) |
|------|-------------|----------------------|
| VID | 0xD7D7 | 0x37d7 |
| PID | 0x0041 | 0x2401 |
| 连接 | 蓝牙 | USB HID |
| 报告长度 | 未知 | 20 字节 |
| 报告格式 | DInput 类似 | 自定义 Vendor |
