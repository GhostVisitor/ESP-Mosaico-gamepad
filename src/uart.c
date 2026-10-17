// ===================================================================================
// UART0 单向发送 —— 实现
// ===================================================================================

#include "ch554.h"
#include "uart.h"

// -----------------------------------------------------------------------------------
// 波特率计算(F_CPU = 16 MHz)
//   UART0 模式 1 的波特率 = (2^SMOD / 32) * Timer1溢出率
//   这里令 Timer1 用"快速时钟" F_CPU/4(=4 MHz),SMOD=1:
//       波特率 = (2/32) * 4MHz/(256-TH1) = F_CPU/(64*(256-TH1))
//   取 256-TH1 = 13  ->  TH1 = 0xF3  ->  19230 bps (误差 0.16%)
// -----------------------------------------------------------------------------------
#define UART_TH1    0xF3      // 19200 bps @16MHz

void UART_init(void) {
  // P3.1 设为推挽输出(UART0 默认 TXD)
  P3_MOD_OC &= ~bTXD;
  P3_DIR_PU |=  bTXD;

  // Timer1 用快速时钟(不设 bTMR_CLK 时为 F_CPU/4)
  T2MOD |= bT1_CLK;

  // Timer1 方式 2(8 位自动重装),作波特率发生器
  TMOD  = (uint8_t)((TMOD & 0x0F) | 0x20);
  TH1   = UART_TH1;
  TL1   = UART_TH1;
  TR1   = 1;

  // 2 倍波特率
  PCON |= SMOD;

  // UART0 方式 1(8 位),不使能接收(REN=0)
  // 只发不收 -> RXD(P3.0) 保持空闲;要接收时把 REN 置 1 即可
  SCON = 0x40;
}

void UART_putc(char c) {
  SBUF = c;
  while (!TI);            // 等发送完成
  TI = 0;                 // 清标志
}

void UART_sendState(uint16_t keys) {
  uint8_t lo = (uint8_t)(keys & 0xFF);
  uint8_t hi = (uint8_t)(keys >> 8);
  uint8_t chk;

  chk = (uint8_t)(UART_FRAME_HEAD0 ^ UART_FRAME_HEAD1 ^ lo ^ hi);

  UART_putc((char)UART_FRAME_HEAD0);
  UART_putc((char)UART_FRAME_HEAD1);
  UART_putc((char)lo);
  UART_putc((char)hi);
  UART_putc((char)chk);
}
