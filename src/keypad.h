// ===================================================================================
// 按键扫描 + 消抖 —— CH552T
// -----------------------------------------------------------------------------------
// 硬件连接(按键一端接引脚,另一端接 GND,按下 = 低电平):
//   KEY_1  -> P1.0    KEY_2  -> P1.1    KEY_3  -> P1.2    KEY_4  -> P1.3
//   KEY_5  -> P1.4    KEY_6  -> P1.5    KEY_7  -> P1.6    KEY_8  -> P1.7
//   KEY_9  -> P3.2    KEY_10 -> P3.3    KEY_11 -> P3.4    KEY_12 -> P3.5
// 所有引脚配置为**内部上拉输入**(准双向口),按下接地拉低。
//
// 键位掩码: bit n 对应 KEY_(n+1),即
//   bit0=KEY_1 ... bit7=KEY_8 (=P1), bit8..bit11 = KEY_9..KEY_12 (=P3)
// ===================================================================================

#pragma once
#include <stdint.h>

// 消抖:连续 N 次采样一致才认可(主循环约 2ms 一轮 -> 约 8ms 消抖)
#define KP_DEBOUNCE_TICKS   4
#define KP_COUNT            12

// 键位掩码(便于应用层使用)
#define K_UP        0x0001      // SW1  D-Pad 上   (P1.0 / KEY_1)
#define K_LEFT      0x0002      // SW2  D-Pad 左   (P1.1 / KEY_2)
#define K_DOWN      0x0004      // SW3  D-Pad 下   (P1.2 / KEY_3)
#define K_RIGHT     0x0008      // SW4  D-Pad 右   (P1.3 / KEY_4)
#define K_Y         0x0010      // SW5  Y          (P1.4 / KEY_5)
#define K_X         0x0020      // SW6  X          (P1.5 / KEY_6)
#define K_A         0x0040      // SW7  A          (P1.6 / KEY_7)
#define K_B         0x0080      // SW8  B          (P1.7 / KEY_8)
#define K_SELECT    0x0100      // SW9  SELECT     (P3.2 / KEY_9)
#define K_START     0x0200      // SW10 START      (P3.3 / KEY_10)
#define K_LT        0x0400      // SW11 LT         (P3.4 / KEY_11)
#define K_RT        0x0800      // SW12 RT         (P3.5 / KEY_12)

// 初始化按键引脚(内部上拉输入)
void KP_init(void);

// 读取一次按键状态,内部完成消抖;返回稳定的键位掩码(bit=1 表示按下)
uint16_t KP_read(void);
