// ===================================================================================
// USB Descriptors and Definitions —— HID 手柄
// -----------------------------------------------------------------------------------
// 需要在 config.h 中定义:
//   USB_VENDOR_ID / USB_PRODUCT_ID / USB_DEVICE_VERSION / USB_MAX_POWER_mA
//   以及各个字符串描述符宏
//
// 端点缓冲放在 xdata 起始处(与 SDK 其它 USB 例程一致),
// 板级 xram_loc = 0x0100,因此缓冲不会和应用变量冲突。
// ===================================================================================

#pragma once
#include <stdint.h>
#include "usb.h"
#include "config.h"

// ===================================================================================
// USB 端点地址与大小
// ===================================================================================
#define EP0_SIZE        8                         // 控制端点(固定 8)
#define EP1_SIZE        16                        // HID IN 端点(报告 9 字节,留点余量)

#define EP_BUF_SIZE(x)  (((x)+2 < 64) ? ((x)+2) : 64)
#define EP0_BUF_SIZE    EP_BUF_SIZE(EP0_SIZE)
#define EP1_BUF_SIZE    EP_BUF_SIZE(EP1_SIZE)

#define EP0_ADDR        0
#define EP1_ADDR        (EP0_ADDR + EP0_BUF_SIZE)

__xdata __at (EP0_ADDR) uint8_t EP0_buffer[EP0_BUF_SIZE];
__xdata __at (EP1_ADDR) uint8_t EP1_buffer[EP1_BUF_SIZE];

// ===================================================================================
// 设备 / 配置描述符
// ===================================================================================
typedef struct _USB_CFG_DESCR_HID {
  USB_CFG_DESCR config;                           // 配置描述符
  USB_ITF_DESCR interface0;                       // 接口描述符
  USB_HID_DESCR hid0;                             // HID 描述符
  USB_ENDP_DESCR ep1IN;                           // 端点 1 (IN, 中断)
} USB_CFG_DESCR_HID, *PUSB_CFG_DESCR_HID;
typedef USB_CFG_DESCR_HID __xdata *PXUSB_CFG_DESCR_HID;

extern __code USB_DEV_DESCR DevDescr;
extern __code USB_CFG_DESCR_HID CfgDescr;

// ===================================================================================
// HID 报告描述符
// ===================================================================================
extern __code uint8_t ReportDescr[];
extern __code uint8_t ReportDescrLen;

#define USB_REPORT_DESCR      ReportDescr
#define USB_REPORT_DESCR_LEN  ReportDescrLen

// ===================================================================================
// 字符串描述符
// ===================================================================================
extern __code uint16_t LangDescr[];
extern __code uint16_t ManufDescr[];
extern __code uint16_t ProdDescr[];
extern __code uint16_t SerDescr[];
extern __code uint16_t InterfDescr[];

#define USB_STR_DESCR_i0    (uint8_t*)LangDescr
#define USB_STR_DESCR_i1    (uint8_t*)ManufDescr
#define USB_STR_DESCR_i2    (uint8_t*)ProdDescr
#define USB_STR_DESCR_i3    (uint8_t*)SerDescr
#define USB_STR_DESCR_i4    (uint8_t*)InterfDescr
#define USB_STR_DESCR_ix    (uint8_t*)SerDescr
