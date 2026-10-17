// ===================================================================================
// 键位 -> 手柄报告 映射 —— 实现
// ===================================================================================

#include "ch554.h"
#include "usb_hid.h"
#include "keypad.h"
#include "gamepad.h"

// 报告字节索引(与 usb_descr.c 的报告描述符一一对应)
//   报告 7 字节: [0]X [1]Y [2]Rx [3]Ry [4]hat+pad [5]btnLo [6]btnHi
#define GP_X        0
#define GP_Y        1
#define GP_RX       2
#define GP_RY       3
#define GP_HAT      4
#define GP_BTN_LO   5
#define GP_BTN_HI   6

static __xdata uint8_t gp_report[GP_REPORT_LEN];
static uint16_t gp_sent;      // 上一次已成功提交的键位掩码
static uint8_t  gp_first;     // 尚未发送过任何报告

void GAMEPAD_init(void) {
  uint8_t i;
  for (i = 0; i < GP_REPORT_LEN; i++) gp_report[i] = 0;

  gp_report[GP_X]  = 128;     // 左摇杆居中
  gp_report[GP_Y]  = 128;
  gp_report[GP_RX] = 128;     // 右摇杆居中(本工程未用)
  gp_report[GP_RY] = 128;
  gp_report[GP_HAT] = 8;      // 苦力帽居中(空状态)

  gp_sent  = 0xFFFF;
  gp_first = 1;
}

void GAMEPAD_update(uint16_t keys) {
  uint16_t btn = 0;
  uint8_t  up, dn, lf, rt, hat;

  if (!gp_first && keys == gp_sent) return;   // 状态没变,不用重发

  up = (keys & K_UP)    ? 1 : 0;
  dn = (keys & K_DOWN)  ? 1 : 0;
  lf = (keys & K_LEFT)  ? 1 : 0;
  rt = (keys & K_RIGHT) ? 1 : 0;

#if GP_DPAD_ALSO_STICK
  // ---- 左摇杆(由方向键满偏驱动) ----
  gp_report[GP_X] = (uint8_t)(lf ? 0 : (rt ? 255 : 128));
  gp_report[GP_Y] = (uint8_t)(up ? 0 : (dn ? 255 : 128));
#endif

  // ---- 苦力帽(8 方向, 8 = 居中) ----
  if      (up) hat = (uint8_t)(lf ? 7 : (rt ? 1 : 0));
  else if (dn) hat = (uint8_t)(lf ? 5 : (rt ? 3 : 4));
  else if (lf) hat = 6;
  else if (rt) hat = 2;
  else         hat = 8;
  gp_report[GP_HAT] = (uint8_t)(hat & 0x0F);   // 高 4 位为填充 = 0

  // ---- 按键(顺序必须按 W3C 标准手柄布局, 见 gamepad.h) ----
  if (keys & K_A)      btn |= 0x0001;   // 1  A
  if (keys & K_B)      btn |= 0x0002;   // 2  B
  if (keys & K_X)      btn |= 0x0004;   // 3  X
  if (keys & K_Y)      btn |= 0x0008;   // 4  Y
  if (keys & K_LT)     btn |= 0x0040;   // 7  LT
  if (keys & K_RT)     btn |= 0x0080;   // 8  RT
  if (keys & K_SELECT) btn |= 0x0100;   // 9  Back
  if (keys & K_START)  btn |= 0x0200;   // 10 Start

  gp_report[GP_BTN_LO] = (uint8_t)(btn & 0xFF);
  gp_report[GP_BTN_HI] = (uint8_t)(btn >> 8);

  // 仅在枚举完成后提交;未成功则不更新 gp_sent,下一轮继续尝试
  if (USB_ENUM_OK && HID_sendReport(gp_report, GP_REPORT_LEN)) {
    gp_sent  = keys;
    gp_first = 0;
  }
}
