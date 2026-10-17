// ===================================================================================
// USB Handler for CH551, CH552 and CH554                                     * v1.5 *
// -----------------------------------------------------------------------------------
// 与 usb_cdc_random 相同的一套底层 USB 中断/枚举处理,只是把
// CDC 的应用层钩子换成了 HID(usb_hid.c 里实现)。
// ===================================================================================

#pragma once
#include <stdint.h>
#include "ch554.h"
#include "usb_descr.h"

// ===================================================================================
// Variables
// ===================================================================================
extern volatile uint8_t  USB_SetupReq, USB_SetupTyp;
extern volatile uint16_t USB_SetupLen;
extern volatile __bit    USB_ENUM_OK;      // 主机下发 SET_CONFIGURATION 后置 1
extern __code uint8_t*   USB_pDescr;

// SETUP 包解析指针(指向 EP0 缓冲)
#define USB_SetupBuf     ((PUSB_SETUP_REQ)EP0_buffer)

// ===================================================================================
// HID 应用层钩子(实现在 usb_hid.c)
// ===================================================================================
void    HID_EP_init(void);                 // 初始化端点
uint8_t HID_control(void);                 // 处理 HID 类 SETUP 请求
void    HID_EP1_IN(void);                  // EP1 IN 传输完成

// ===================================================================================
// 钩子绑定
// ===================================================================================
#define USB_INIT_endpoints      HID_EP_init     // 端点初始化钩子
#define USB_CLASS_SETUP_handler HID_control     // HID 类请求处理钩子

#define EP0_SETUP_callback  USB_EP0_SETUP
#define EP0_IN_callback     USB_EP0_IN
#define EP0_OUT_callback    USB_EP0_OUT
#define EP1_IN_callback     HID_EP1_IN

// ===================================================================================
// Functions
// ===================================================================================
void USB_init(void);
void USB_interrupt(void);
void USB_EP0_copyDescr(uint8_t len);
