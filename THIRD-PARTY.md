# 第三方代码归属声明（THIRD-PARTY NOTICES）

本工程站在别人的肩膀上，以下代码并非原创，**再分发时请保留其原始许可与署名**。

---

## 1. CH55x USB 底层驱动

| 文件 | 来源 | 是否修改 |
|---|---|:--:|
| `src/usb.h` | Stefan Wagner 的 ch554_sdcc 开源库 | 未修改（与上游逐字节相同）|
| `src/usb_handler.c` | 同上（经 UNIT-Electronics-MX 整理为 `unit_ch55x` PlatformIO 平台）| 未修改（与上游逐字节相同）|
| `src/usb_handler.h` | 同上 | **已修改**：把 CDC 应用层钩子换成了 HID 钩子 |

**原始许可原文（摘自库内 LICENSE 文件）**：

```
The original code is licensed under the Creative Commons Attribution-ShareAlike 3.0
Unported License (CC BY-SA 3.0). For more details, visit:
http://creativecommons.org/licenses/by-sa/3.0/

Modifications and additional code in this file are licensed under the GNU General
Public License v3.0 (GPL-3.0). For more information, see:
https://www.gnu.org/licenses/gpl-3.0.en.html
```

- 原作者：**Stefan Wagner**（wagiminator）
- 集成方：**UNIT-Electronics-MX**（PlatformIO 平台 `unit_ch55x`）
- 许可：**CC BY-SA 3.0**（原始代码）/ **GPL-3.0**（修改与新增部分）
- 许可全文：[`LICENSE-CC-BY-SA-3.0.txt`](LICENSE-CC-BY-SA-3.0.txt) / [`LICENSE`](LICENSE)

> **所以本工程整体以 GPL-3.0 发布**：固件把上面几个文件链接成了同一个二进制，
> 按 GPL 的规定，整个工程（含 `firmware/*.hex`、`*.bin` 等编译产物）必须是 GPL-3.0。
> 应用层文件在 GPL-3.0 之外**额外**提供 MIT 许可（见 [`LICENSE-MIT`](LICENSE-MIT)）。
> GPL 允许商用，但二次分发时你必须同样开放源码并保留本声明。
> 上面三个文件**不适用** MIT 许可，别单独把它们当 MIT 用。

---

## 2. 芯片厂商资料

- `src/ch554.h`、`system.h`、`gpio.h`、`delay.*`、`i2c.*`、`oled.*` 等寄存器/外设头文件与库
  **并未包含在本工程源码目录中**，它们随 PlatformIO 平台 `unit_ch55x` 一起提供，
  版权归 **WCH（南京沁恒微电子股份有限公司）** 及平台作者所有。
- 本工程仅通过 `#include` 使用，未做修改。
- 芯片手册：`CH551/CH552 Datasheet (CH552DS1.PDF)`，来自 WCH 官网 <https://www.wch-ic.com>。

---

## 3. 本文档中引用的手册原文

`README.md`、`docs/踩坑记录.md` 中引用的 CH552 手册原文（如 `CLOCK_CFG` 寄存器位定义、
引脚功能表），版权归 **WCH** 所有，此处仅作技术说明与合理引用。

---

## 4. 其它可选工具（本工程未包含，仅在文档中推荐）

| 名称 | 用途 | 许可 |
|---|---|---|
| [ViGEmBus](https://github.com/nefarius/ViGEmBus) | PC 侧把 HID 手柄桥接成虚拟 XInput | BSD-3-Clause |
| [x360ce](https://github.com/x360ce/x360ce) | 同上（放在游戏目录） | LGPL |
| [hidapi](https://github.com/libusb/hidapi) | `tools/test_hid.py` 读取 HID 报告所需 | BSD-3-Clause |
| [SDL_GameControllerDB](https://github.com/gabomdq/SDL_GameControllerDB) | 手柄映射数据库 | zlib |
