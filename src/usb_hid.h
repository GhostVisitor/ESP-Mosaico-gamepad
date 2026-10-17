// ===================================================================================
// HID 设备层接口
// ===================================================================================

#pragma once
#include <stdint.h>
#include "usb_handler.h"

// 初始化 USB-HID
void    HID_init(void);

// 发送 HID 输入报告 (返回 1 已提交 / 0 忙跳过)
uint8_t HID_sendReport(__xdata uint8_t* buf, uint8_t len);
