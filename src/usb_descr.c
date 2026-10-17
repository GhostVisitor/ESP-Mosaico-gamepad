// ===================================================================================
// USB Descriptors —— CH552T 12 键面板 / 虚拟 XBOX360 手柄
// ===================================================================================

#include "usb_descr.h"

// ===================================================================================
// HID 报告描述符 (Game Pad, 报告长度 7 字节)
// -----------------------------------------------------------------------------------
// ★ 按键位置必须符合 W3C《标准手柄》布局, 浏览器/测试网站/游戏才能正确对号:
//     位置: 0  1  2  3  4  5  6  7  8     9
//     名称: A  B  X  Y  LB RB LT RT Back  Start
//
// 报告字节布局(与描述符顺序一一对应):
//   [0] X    左摇杆 X   (0..255, 中位 128)
//   [1] Y    左摇杆 Y   (0..255, 中位 128)
//   [2] Rx   右摇杆 X   (本工程未用, 恒 128)
//   [3] Ry   右摇杆 Y   (本工程未用, 恒 128)
//   [4] 低 4 位 = 苦力帽(POV 方向 0..7, 8=居中); 高 4 位为填充
//   [5] 按键 1..8  (bit0=A, bit1=B, bit2=X, bit3=Y, bit4=LB, bit5=RB,
//                   bit6=LT, bit7=RT)
//   [6] 按键 9..16 (bit0=Back, bit1=Start, 其余本工程未用)
//
// 注: 扳机 LT/RT 用**数字按键**表示(不用 Z/Rz 模拟轴), 这样才与标准手柄布局一致。
// ===================================================================================
__code uint8_t ReportDescr[] = {
  // ---- 应用集合 (Game Pad) ----
  0x05, 0x01,           //   Usage Page (Generic Desktop)
  0x09, 0x05,           //   Usage (Game Pad)
  0xA1, 0x01,           //   Collection (Application)

  // ---- 左摇杆 X / Y ----
  0x09, 0x30,           //     Usage (X)
  0x09, 0x31,           //     Usage (Y)
  0x15, 0x00,           //     Logical Minimum (0)
  0x26, 0xFF, 0x00,     //     Logical Maximum (255)
  0x75, 0x08,           //     Report Size (8)
  0x95, 0x02,           //     Report Count (2)
  0x81, 0x02,           //     Input (Data,Var,Abs)

  // ---- 右摇杆 Rx / Ry (预留) ----
  0x09, 0x33,           //     Usage (Rx)
  0x09, 0x34,           //     Usage (Ry)
  0x15, 0x00,           //     Logical Minimum (0)
  0x26, 0xFF, 0x00,     //     Logical Maximum (255)
  0x75, 0x08,           //     Report Size (8)
  0x95, 0x02,           //     Report Count (2)
  0x81, 0x02,           //     Input (Data,Var,Abs)

  // ---- 苦力帽 / 方向键 (Hat switch) ----
  0x09, 0x39,           //     Usage (Hat switch)
  0x15, 0x00,           //     Logical Minimum (0)
  0x25, 0x07,           //     Logical Maximum (7)
  0x35, 0x00,           //     Physical Minimum (0)
  0x46, 0x3B, 0x01,     //     Physical Maximum (315)
  0x65, 0x14,           //     Unit (Eng Rot: degrees)
  0x75, 0x04,           //     Report Size (4)
  0x95, 0x01,           //     Report Count (1)
  0x81, 0x42,           //     Input (Data,Var,Abs,Null State)
  0x65, 0x00,           //     Unit (None)
  0x75, 0x04,           //     Report Size (4)   -- 高 4 位填充
  0x95, 0x01,           //     Report Count (1)
  0x81, 0x01,           //     Input (Const)

  // ---- 按键 1..16 (本工程只用 1..10) ----
  0x05, 0x09,           //     Usage Page (Button)
  0x19, 0x01,           //     Usage Minimum (1)
  0x29, 0x10,           //     Usage Maximum (16)
  0x15, 0x00,           //     Logical Minimum (0)
  0x25, 0x01,           //     Logical Maximum (1)
  0x75, 0x01,           //     Report Size (1)
  0x95, 0x10,           //     Report Count (16)
  0x81, 0x02,           //     Input (Data,Var,Abs)

  0xC0                  //   End Collection
};

__code uint8_t ReportDescrLen = (uint8_t)sizeof(ReportDescr);

// ===================================================================================
// 设备描述符
// ===================================================================================
__code USB_DEV_DESCR DevDescr = {
  .bLength            = sizeof(DevDescr),       // 18 字节
  .bDescriptorType    = USB_DESCR_TYP_DEVICE,   // 0x01
  .bcdUSB             = 0x0110,                 // USB 1.1
  .bDeviceClass       = 0,                      // 由接口定义类别
  .bDeviceSubClass    = 0,
  .bDeviceProtocol    = 0,
  .bMaxPacketSize0    = EP0_SIZE,               // 8
  .idVendor           = USB_VENDOR_ID,
  .idProduct          = USB_PRODUCT_ID,
  .bcdDevice          = USB_DEVICE_VERSION,
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 3,
  .bNumConfigurations = 1
};

// ===================================================================================
// 配置描述符 (1 接口 + HID + 端点 1 IN,共 32 字节)
// ===================================================================================
__code USB_CFG_DESCR_HID CfgDescr = {

  .config = {
    .bLength            = sizeof(USB_CFG_DESCR),
    .bDescriptorType    = USB_DESCR_TYP_CONFIG,
    .wTotalLength       = sizeof(CfgDescr),
    .bNumInterfaces     = 1,
    .bConfigurationValue= 1,
    .iConfiguration     = 0,
    .bmAttributes       = 0x80,                   // 总线供电,不支持远程唤醒
    .MaxPower           = USB_MAX_POWER_mA / 2    // 单位 2mA
  },

  .interface0 = {
    .bLength            = sizeof(USB_ITF_DESCR),
    .bDescriptorType    = USB_DESCR_TYP_INTERF,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,
    .bInterfaceClass    = USB_DEV_CLASS_HID,      // 0x03
    .bInterfaceSubClass = 0,                      // 非启动设备
    .bInterfaceProtocol = 0,                      // 无
    .iInterface         = 4
  },

  .hid0 = {
    .bLength            = sizeof(USB_HID_DESCR),
    .bDescriptorType    = USB_DESCR_TYP_HID,      // 0x21
    .bcdHID             = 0x0110,                 // HID 1.1
    .bCountryCode       = 0,
    .bNumDescriptors    = 1,
    .bDescriptorTypeX   = USB_DESCR_TYP_REPORT,   // 0x22
    .wDescriptorLength  = sizeof(ReportDescr)
  },

  .ep1IN = {
    .bLength            = sizeof(USB_ENDP_DESCR),
    .bDescriptorType    = USB_DESCR_TYP_ENDP,
    .bEndpointAddress   = USB_ENDP_ADDR_EP1_IN,   // 0x81
    .bmAttributes       = USB_ENDP_TYPE_INTER,    // 中断传输
    .wMaxPacketSize     = EP1_SIZE,
    .bInterval          = 1                       // 1ms 轮询
  }
};

// ===================================================================================
// 字符串描述符
// ===================================================================================

// 语言 (索引 0)
__code uint16_t LangDescr[] = {
  ((uint16_t)USB_DESCR_TYP_STRING << 8) | sizeof(LangDescr), 0x0409 };  // 美式英语

// 厂商 (索引 1)
__code uint16_t ManufDescr[] = {
  ((uint16_t)USB_DESCR_TYP_STRING << 8) | sizeof(ManufDescr), MANUFACTURER_STR };

// 产品 (索引 2)
__code uint16_t ProdDescr[] = {
  ((uint16_t)USB_DESCR_TYP_STRING << 8) | sizeof(ProdDescr), PRODUCT_STR };

// 序列号 (索引 3)
__code uint16_t SerDescr[] = {
  ((uint16_t)USB_DESCR_TYP_STRING << 8) | sizeof(SerDescr), SERIAL_STR };

// 接口 (索引 4)
__code uint16_t InterfDescr[] = {
  ((uint16_t)USB_DESCR_TYP_STRING << 8) | sizeof(InterfDescr), INTERFACE_STR };
