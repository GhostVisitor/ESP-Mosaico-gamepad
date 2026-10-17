/*
 * ============================================================================
 *  CH552T —— 12 键面板 / USB 虚拟 XBOX360 手柄 + 串口上报
 * ----------------------------------------------------------------------------
 *  功能:
 *    1) 扫描 12 个按键(内部上拉,按下接地,带消抖)
 *    2) 通过 UART0(P3.1 = TXD, 19200 8N1)把键位掩码上报给上级单片机
 *    3) 同时通过 USB 虚拟成一个 XBOX360 风格的手柄(HID 游戏控制器)
 *
 *  按键 -> 引脚:
 *    KEY_1..KEY_8  -> P1.0..P1.7
 *    KEY_9..KEY_12 -> P3.2, P3.3, P3.4, P3.5
 *  串口:
 *    P3.1 = TXD (P3.0 = RXD, 本工程只发不收)
 *
 *  注意:
 *    - 使用 16 MHz 内部振荡器(USB 需要),不要随意改 F_CPU。
 * ============================================================================
 */

#include "config.h"
#include "system.h"
#include "delay.h"
#include "ch554.h"
#include "keypad.h"
#include "uart.h"
#include "gamepad.h"
#include "usb_hid.h"

// 主循环周期与串口心跳周期
//   注: 实测本平台的 DLY_ms(2) 实际约 4ms(SDK 的 DLY_us 标定偏保守),
//       因此真实心跳周期约 400ms、按键消抖约 16ms。
//       上位机判"面板掉线"建议用 1~2s 超时。
#define LOOP_MS          2
#define HEARTBEAT_MS     200
#define HEARTBEAT_TICKS  (HEARTBEAT_MS / LOOP_MS)

// ---- USB 中断服务程序(SDCC 要求函数原型出现在 main 所在文件)----
void USB_interrupt(void);
void USB_ISR(void) __interrupt(INT_NO_USB) {
  USB_interrupt();
}

// ============================================================================
void main(void) {
  uint16_t keys;
  uint16_t last_keys = 0xFFFF;    // 保证第一次一定会上报
  uint16_t hb = 0;

  CLK_config();                   // SDK 的时钟配置(16MHz)
  DLY_ms(5);                      // 等待时钟稳定

#if F_CPU == 16000000
  // ★★ 必须的修正 (CH552 手册 8.2 / 8.3) ★★
  //   SDK 的 CLK_config() 在 F_CPU=16MHz 时会写入 CLOCK_CFG = 0x45,即置起
  //   bit6 = bOSC_EN_XT。而手册对 bit6 的定义是:
  //       "1: P1.2/P1.3 pin used as XI/XO and the oscillator enabled."
  //   也就是 P1.2 = XI、P1.3 = XO 会被**晶振外设**占用,失去 GPIO 上拉
  //   (实测空闲电平只有 1.67V ≈ VCC/2),且 XO 一直在振荡 -> 读回来疯狂抖动!
  //
  //   正确配置(仍为 16MHz,内部振荡器,不占 P1.2/P1.3):
  //     bit7 bOSC_EN_INT   = 1  -> 用内部 24MHz 振荡器
  //     bit6 bOSC_EN_XT    = 0  -> 关外部晶振(释放 P1.2/P1.3 作 GPIO)
  //     bit4 bROM_CLK_FAST = 0  -> Fosc(24MHz) >= 16MHz 用普通档
  //     [2:0] MASK_SYS_CK_SEL = 101b -> Fsys = Fpll/6 = 96/6 = 16MHz
  //   即 CLOCK_CFG = 1000 0101b = 0x85
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xAA;
  CLOCK_CFG = 0x85;
  SAFE_MOD = 0x00;
#endif

  KP_init();                      // 按键引脚: 内部上拉输入
  UART_init();                    // P3.1 = TXD, 19200 8N1
  GAMEPAD_init();

  // 显式复位所有"引脚复用"位(只保留 USB 引脚使能):
  //   - bUSB_IO_EN = 1  -> P3.6/P3.7 用作 USB D-/D+
  //   - 清 bUART0_PIN_X -> UART0 保持在默认引脚 P3.0/P3.1
  //     (否则 bootloader 若把 RXD_/TXD_ 复用到 P1.2/P1.3,
  //      会和按键冲突, 且 P3.1 上根本没有串口输出)
  //   - 清 bT2_PIN_X / bT2EX_PIN_X -> P1.1/P1.4 保持纯 GPIO
  PIN_FUNC = bUSB_IO_EN;
  HID_init();                     // 初始化 USB(内部会 EA = 1)

  while (1) {
    keys = KP_read();             // 读取并消抖,得到稳定键位
    GAMEPAD_update(keys);         // 更新 USB 手柄报告(有变化才发)

    if (keys != last_keys) {      // 状态变化 -> 立即上报
      UART_sendState(keys);
      last_keys = keys;
      hb = 0;
    } else if (++hb >= HEARTBEAT_TICKS) {   // 状态不变 -> 定期心跳
      UART_sendState(keys);
      hb = 0;
    }

    DLY_ms(LOOP_MS);
  }
}
