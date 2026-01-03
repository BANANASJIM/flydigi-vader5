# 项目架构

## 目录结构

```
flydigi-vader5/
├── driver/                    # 内核驱动
│   ├── hid-flydigi.c
│   ├── Makefile
│   ├── dkms.conf
│   └── 99-flydigi.rules
├── research/                  # 研究文档
│   ├── vader3-analysis.md
│   ├── protocol-24g.md
│   └── architecture.md
├── openspec/                  # 规范管理
│   ├── project.md
│   ├── AGENTS.md
│   └── changes/
├── scripts/                   # 构建/安装脚本
│   ├── install.sh
│   └── uninstall.sh
├── .github/                   # CI/CD (未来)
│   └── workflows/
│       └── build.yml
├── CLAUDE.md
├── README.md
└── LICENSE
```

## 内核驱动结构

```c
// hid-flydigi.c

// 设备 ID 表
static const struct hid_device_id flydigi_devices[] = {
    { HID_USB_DEVICE(0x37d7, 0x2401) },  // Vader 5 Pro 2.4G
    { }
};

// 驱动入口
static struct hid_driver flydigi_driver = {
    .name = "flydigi",
    .id_table = flydigi_devices,
    .input_configured = flydigi_input_configured,
    .raw_event = flydigi_raw_event,
};

module_hid_driver(flydigi_driver);
```

## CI 规划 (未来)

```yaml
# .github/workflows/build.yml
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install kernel headers
        run: sudo apt-get install linux-headers-$(uname -r)
      - name: Build module
        run: make -C driver
```

## 开发流程

1. 协议逆向 → research/protocol-24g.md
2. 驱动实现 → driver/hid-flydigi.c
3. 本地测试 → insmod / rmmod
4. DKMS 打包 → dkms add/build/install
5. CI 验证 → GitHub Actions
