// ===================================================================================
// HID 设备层 —— CH551/CH552/CH554 (基于 Stefan Wagner ch554_sdcc 库)
// -----------------------------------------------------------------------------------
// 提供的接口:
//   HID_init()                     初始化 USB(HID)
//   HID_sendReport(buf, len)       发送一份 HID 输入报告(非阻塞)
//
// 与 CDC 版的区别:发送**不阻塞**。若上一包还没发完,本次直接跳过并返回 0,
// 由应用层下一轮再试。这样即便主机停止轮询,主循环也不会被卡死。
// ===================================================================================

#include "ch554.h"
#include "usb.h"
#include "usb_hid.h"
#include "usb_descr.h"

// ===================================================================================
// 变量
// ===================================================================================
volatile __bit HID_EP1_writeBusyFlag = 0;   // EP1 上传忙标志

// 最近一次已写入的报告(供 GET_REPORT 使用,可选)
static __xdata uint8_t HID_lastReport[EP1_SIZE];
static uint8_t         HID_lastReportLen = 0;

// ===================================================================================
// 前端接口
// ===================================================================================

// 初始化 USB-HID
void HID_init(void) {
  USB_init();                     // 内含端点初始化(USB_INIT_endpoints -> HID_EP_init)
  UEP1_T_LEN = 0;
}

// 发送 HID 输入报告
//   返回 1: 已提交发送; 返回 0: 上一包未发完,本次跳过(稍后重试)
uint8_t HID_sendReport(__xdata uint8_t* buf, uint8_t len) {
  uint8_t i;

  if (HID_EP1_writeBusyFlag) return 0;      // 忙 -> 跳过,不阻塞
  if (len > EP1_SIZE) len = EP1_SIZE;       // 长度保护

  for (i = 0; i < len; i++) {
    EP1_buffer[i]  = buf[i];                // 拷贝报告到端点缓冲
    HID_lastReport[i] = buf[i];
  }
  HID_lastReportLen = len;

  UEP1_T_LEN = len;                         // 设置本次上传长度
  HID_EP1_writeBusyFlag = 1;                // 置忙标志
  UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;  // 触发上传
  return 1;
}

// ===================================================================================
// HID 专用 USB 处理函数
// ===================================================================================

// 初始化/复位 HID 端点
void HID_EP_init(void) {
  UEP1_DMA    = (uint16_t)EP1_buffer;             // EP1 数据地址
  UEP1_CTRL   = bUEP_AUTO_TOG                     // 自动翻转同步标志
              | UEP_T_RES_NAK;                    // 默认 IN 返回 NAK
  UEP4_1_MOD  = bUEP1_TX_EN;                      // 使能 EP1 发送
  UEP1_T_LEN  = 0;
  HID_EP1_writeBusyFlag = 0;
}

// 处理 HID 类 SETUP 请求
uint8_t HID_control(void) {
  uint8_t i;
  switch (USB_SetupReq) {
    case HID_GET_REPORT:                          // 0x01 主机读取当前报告
      if (HID_lastReportLen > EP0_BUF_SIZE) HID_lastReportLen = EP0_BUF_SIZE;
      for (i = 0; i < HID_lastReportLen; i++) EP0_buffer[i] = HID_lastReport[i];
      return HID_lastReportLen;

    case HID_GET_IDLE:                            // 0x02
      EP0_buffer[0] = 0;
      return 1;

    case HID_GET_PROTOCOL:                        // 0x03
      EP0_buffer[0] = 1;                          // 报告协议
      return 1;

    case HID_SET_IDLE:                            // 0x0A 主机设置空闲频率,忽略
      return 0;

    case HID_SET_PROTOCOL:                        // 0x0B 忽略
      return 0;

    default:
      return 0xff;                                // 不支持 -> STALL
  }
}

// 端点 1 IN 传输完成
void HID_EP1_IN(void) {
  UEP1_T_LEN = 0;                                 // 没有更多数据
  UEP1_CTRL  = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
  HID_EP1_writeBusyFlag = 0;                      // 清忙标志
}
