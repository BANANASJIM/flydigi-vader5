# Vader 5 Pro 2.4G USB 协议

## 设备信息

- VID: 0x37d7
- PID: 0x2401

### USB 接口布局 (lsusb -v)

| Interface | Class | Endpoints | 用途 |
|-----------|-------|-----------|------|
| 0 | Vendor (0xff/0x5d Xbox) | EP1 IN 20B, EP5 OUT 8B | **主输入 + 震动** |
| 1 | HID | EP2 IN 32B, EP6 OUT 32B | **配置命令/响应** |
| 2 | HID | EP2 IN 32B | 扩展输入 (测试模式) |
| 3 | HID | EP4 IN 64B | 键盘模式 |

> **注意**: 命令包长度是 **32 字节**，不是 64 字节 (2025-01-01 抓包确认)。

---

## 输入报告 (Device → Host)

### Interface 0 / EP1 IN: 主输入 (20 字节)

```
Offset  Size  Description
------  ----  -----------
0       1     报告 ID (0x00)
1       1     子类型 (0x14 = 2.4G)
2       1     misc: D-Pad + Start/Select/L3/R3
3       1     buttons: LB/RB/Home + ABXY
4       1     左扳机 LT (0-255)
5       1     右扳机 RT (0-255)
6-7     2     左摇杆 X (int16 LE, 左=-32768, 右=+32767)
8-9     2     左摇杆 Y (int16 LE, 下=-32768, 上=+32767)
10-11   2     右摇杆 X (int16 LE)
12-13   2     右摇杆 Y (int16 LE)
14-19   6     未使用
```

**byte[2] misc 位域:**

| Bit | 功能 |
|-----|------|
| 0 | D-Pad 上 |
| 1 | D-Pad 下 |
| 2 | D-Pad 左 |
| 3 | D-Pad 右 |
| 4 | Start |
| 5 | Select |
| 6 | L3 |
| 7 | R3 |

**byte[3] buttons 位域:**

| Bit | 功能 |
|-----|------|
| 0 | LB |
| 1 | RB |
| 2 | Home |
| 3 | (未使用) |
| 4 | A |
| 5 | B |
| 6 | X |
| 7 | Y |

### Interface 1 / EP2 IN: Extended Input + IMU (32 bytes, requires test mode)

> **Note**: Extended input data is returned via Interface 1 (not Interface 2). Use hidraw to read.

Magic: `5a a5 ef`

```
Offset  Size  Description
------  ----  -----------
0-2     3     Magic (5a a5 ef)
3-4     2     Left stick X (int16 LE)
5-6     2     Left stick Y (int16 LE, invert: Y = -raw_Y)
7-8     2     Right stick X (int16 LE)
9-10    2     Right stick Y (int16 LE, invert: Y = -raw_Y)
11      1     Button field 1 (D-Pad + A/B/Select/X)
12      1     Button field 2 (Y/Start/LB/RB/L3/R3)
13      1     Ext buttons 1 (C/Z/M1-M4/LM/RM)
14      1     Ext buttons 2 (O/Home)
15      1     Left trigger LT (0-255)
16      1     Right trigger RT (0-255)
17-18   2     Gyro X (int16 LE)
19-20   2     Gyro Y (int16 LE)
21-22   2     Gyro Z (int16 LE)
23-24   2     Accel X (int16 LE, 4096 = 1g)
25-26   2     Accel Y (int16 LE)
27-28   2     Accel Z (int16 LE)
29-31   3     Reserved
```

**byte[11] Button field 1:**

| Bit | Function |
|-----|----------|
| 0 | D-Pad Up |
| 1 | D-Pad Down |
| 2 | D-Pad Left |
| 3 | D-Pad Right |
| 4 | A |
| 5 | B |
| 6 | Select |
| 7 | X |

**byte[12] Button field 2:**

| Bit | Function |
|-----|----------|
| 0 | Y |
| 1 | Start |
| 2 | LB |
| 3 | RB |
| 4 | (unused) |
| 5 | (unused) |
| 6 | L3 |
| 7 | R3 |

**byte[13] Ext buttons 1:**

| Bit | Button |
|-----|--------|
| 0 | C |
| 1 | Z |
| 2 | M1 |
| 3 | M2 |
| 4 | M3 |
| 5 | M4 |
| 6 | LM |
| 7 | RM |

**byte[14] Ext buttons 2:**

| Bit | Button |
|-----|--------|
| 0 | O |
| 1 | Home |

---

## Xbox Rumble (Interface 0 / EP5 OUT)

### Xbox 360 Format (8 bytes) - SUPPORTED

```
Offset  Value
------  -----
0       0x00
1       0x08
2       0x00
3       left_motor  (0-255)
4       right_motor (0-255)
5-7     0x00
```

### Xbox One Format (13 bytes) - NOT SUPPORTED

The controller does not respond to Xbox One GIP protocol rumble commands.
Trigger rumble motors may require Flydigi proprietary protocol (EP6 OUT).

Source: [Linux xpad.c](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c)

---

## Flydigi 命令 (Interface 1 / EP6 OUT)

通过 Interface 1 / EP6 OUT 发送，**32 字节**，Magic: `5a a5`

### 初始化握手 (必需)

设备插入后必须发送初始化序列，否则配置命令无响应：

```
CMD 0x01: 5a a5 01 02 03  (设备信息查询)
CMD 0xa1: 5a a5 a1 02 a3  (MAC/序列号)
CMD 0x02: 5a a5 02 02 04  (配置读取)
CMD 0x04: 5a a5 04 02 06  (配置数据)
```

校验和格式: byte[2] + byte[3] + byte[4] (溢出截断)

### 命令列表

| 命令 | 功能 | 说明 |
|------|------|------|
| `11 07` | 测试模式 | byte[5]: 01=开, 00=关 |
| `12 06` | 振动测试 | byte[4-5]: 马达强度 |
| `a2 03` | 切换配置档 | byte[4]: 档位 (0-3) |
| `a4 06` | 单按键映射 | 设置单个按键 |
| `a5 17` | 批量配置 | 按键映射 + 振动设置 |
| `a6 04` | 保存 | 写入板载存储 |
| `a8 06` | LED 单项设置 | |
| `a9 17` | LED 批量配置 | 模式/亮度/颜色 |

### 测试模式 (11 07)

```
5a a5 11 07 ff XX ff ff ff YY 00...
               ^^          ^^
               开关        模式
XX: 01=开, 00=关
YY: 15=测试, 14=普通
```

### 配置档切换 (a2 03)

```
5a a5 a2 03 XX YY 00...
            ^^  ^^
            档位 校验和
XX: 00-03 (配置档 1-4)
YY: 0xa5 + XX
```

### 振动测试 (12 06)

**发送:**
```
5a a5 12 06 XX XX 00...
            ^^ ^^
            左 右马达强度
ff ff = 开, 00 00 = 关
```

**响应:**
```
5a a5 12 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 13
         ^^                                                                                 ^^
         status (01=成功)                                                                  校验和
```

---

## 配置结构

### 按键映射编码

用于 `a4 06` 和 `a5 17` 命令的目标按键值：

| 值 | 按键 |
|----|------|
| `00` | D-Up |
| `01` | D-Right |
| `02` | D-Down |
| `03` | D-Left |
| `04` | A |
| `05` | B |
| `06` | X |
| `07` | Y |
| `08` | Select (待确认) |
| `09` | Start |
| `0a` | LB |
| `0b` | RB |
| `0c` | LT |
| `0d` | RT |
| `0e` | L3 |
| `0f` | R3 |
| `ff` | 无映射 |

### 扩展按键索引 (源按键)

| 索引 | 按键 |
|------|------|
| 0 | C |
| 1 | Z |
| 2 | M1 |
| 3 | M2 |
| 4 | M3 |
| 5 | M4 |
| 6 | LM |
| 7 | RM |
| 8 | O |

### 振动配置 (a5 17, byte[5]=04)

```
5a a5 a5 17 00 04 19 19 00 00 00 00 3c ff XX 00 3c ff XX 00...
                                       ^^          ^^
                                       左马达%     右马达%
XX = 十进制百分比 (19=25%, 32=50%, 64=100%)
```

---

## LED 配置 (a9 17)

### 子类型 (byte[4])

| 值 | 用途 |
|----|------|
| `00` | 模式/亮度配置 |
| `01` | 颜色层 1 |
| `02` | 颜色层 2 |
| `03` | 颜色层 3 |
| `04` | 颜色层 4 |
| `05` | 颜色层 5 |

### 模式配置 (byte[4]=00)

```
5a a5 a9 17 00 ... XX ... YY ... ZZ ...
                   ^^     ^^     ^^
                   byte9  byte10 byte11
byte[9]:  LED 模式
byte[10]: 循环时间 (十进制)
byte[11]: 亮度 (十进制)
byte[13]: 模式参数
```

**LED 模式值:**

| byte[9] | byte[13] | 模式 |
|---------|----------|------|
| `00` | `05` | 常亮 |
| `00` | `06` | 关闭 |
| `01` | `02` | 呼吸 |
| `02` | `03` | 渐变 |
| `09` | `07` | 循环 |
| `0a` | `01` | 流光 |

### 颜色配置

**颜色格式:**
- byte[4]=00: BRG 格式
- byte[4]=01+: GRB 格式 (部分场景可能为 RGB)

**颜色示例:**

| 颜色 | BRG (byte[4]=00) | GRB (byte[4]=01) |
|------|------------------|------------------|
| 红色 (255,0,0) | `00 ff 00` | `00 ff 00` |
| 绿色 (0,255,0) | `00 00 ff` | `ff 00 00` |
| 蓝色 (0,0,255) | `ff 00 00` | `00 00 ff` |

每个颜色层包含多个 LED 的颜色数据 (3 字节/LED)。

---

## TODO

- [x] Gyro/Accel data parsing (done: see Extended Input section)
- [ ] Read onboard config commands
- [ ] LED color format full verification
- [ ] Macro/combo config
