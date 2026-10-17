# 预编译固件说明

本目录存放可直接烧写的固件镜像，**无需安装编译器**。

| 文件 | 大小 | 说明 |
|---|:--:|---|
| `ESP-Mosaico_gamepad_v1.0.hex` | 9061 B | **推荐**，Intel HEX 格式，WCHISPTool / vnproch55x 直接烧写 |
| `ESP-Mosaico_gamepad_v1.0.bin` | 3709 B | 二进制镜像（占用 14336 B Flash 的 26%） |

> 这两个文件由本工程源码编译产生（PlatformIO + SDCC 4.4.0，零告警）。
> 修改源码后请自行 `pio run` 重新生成。

---

## 一、进入 Bootloader

CH552 上电时如果检测到 **`D+`（P3.6）被上拉**，就会停留在内置 bootloader 里等待下载：

1. 断电
2. 用 **10 kΩ** 把 **`D+`（P3.6）拉到 3.3 V 或 5 V**
3. 上电（或按一下复位）
4. 此时电脑上会出现 WCH 的 USB 下载设备

> 若板上已有这个 10 kΩ 上拉，则每次上电都会停在 bootloader（需要断开才能跑固件）；
> 常见做法是串一个按键或跳线，只在烧录时接上。

---

## 二、Windows：WCHISPTool（官方，最省事）

1. 到 WCH 官网下载 **WCHISPTool**（沁恒 USB 下载工具）
2. 按上面步骤让芯片进入 bootloader
3. 打开 WCHISPTool → 芯片系列选 **CH55x** → 器件选 **CH552**
4. 选择 `ESP-Mosaico_gamepad_v1.0.hex`
5. 点 **下载**，成功后断电、断开 D+ 上拉、重新上电即可运行

---

## 三、Windows：命令行 vnproch55x

按上面步骤进入 bootloader 后：

```powershell
# 全工程已集成上传器，直接：
pio run -t upload

# 或手动调用（bootloader 状态下）
vnproch55x.exe -t CH552 -c 3 ESP-Mosaico_gamepad_v1.0.hex
```

> `-c 3` 对应 CH552 的 boot 配置。不同版本参数可能略有差异，以 `vnproch55x -h` 为准。

---

## 四、Linux / macOS：chprog.py

`examples/usb/prog` 目录下有 WCH 官方的 Python 烧录脚本 `chprog.py`（需 `pyusb`）：

```bash
pip install pyusb
python chprog.py ESP-Mosaico_gamepad_v1.0.bin
```

---

## 五、烧录后自检

1. **USB 手柄**：`Win+R` → `joy.cpl` → 应看到 `HID-compliant game controller`，逐个按键有响应
2. **串口**：主控 RX 接 `P3.1`、共地，19200 8N1，应能收到 `A5 5A .. .. ..` 帧
   （可用本工程 `tools/test_serial.ps1` 直接解码成键名）

---

## 校验值（SHA-256）

```
ESP-Mosaico_gamepad_v1.0.hex
016d7d912349aa5e22040374242da4302dbf6828a8bfefddcd7d958188857d68

ESP-Mosaico_gamepad_v1.0.bin
0da51d0ca0e193339abe88cacdaa30b85e5f45a59273ac12ea97a6883f26175b
```
