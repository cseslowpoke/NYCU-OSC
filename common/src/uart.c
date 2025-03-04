#include "uart.h"
#include "utils.h"

void uart_init() {
  // setting gpio 14 and 15 to alternative function 5 (mini uart)
  *GPFSEL1 = 0x12000;

  // setting gpio Pull-up/down control register
  *GPPUD = 0x1;
  delay_cycles(150);
  *GPPUDCLK0 = 1 << 15;
  delay_cycles(150);
  *GPPUD = 0;
  *GPPUDCLK0 = 0;

  // setting mini uart registers
  *AUX_ENABLES = 0;
  *AUX_MU_CNTL = 0;
  *AUX_MU_IER = 0;
  *AUX_MU_LCR = 3;
  *AUX_MU_MCR = 0;
  *AUX_MU_BAUD = 270;
  *AUX_MU_IIR = 6;
  *AUX_MU_CNTL = 3;

  return;
}

void uart_send(char c) {
  while (!UART_CAN_WRITE())
    ;
  (*AUX_MU_IO) = c;
}

char uart_recv() {
  while (!UART_CAN_READ())
    ;
  return (char)(*AUX_MU_IO);
}

void uart_send_string(char *str) {
  while (*str) {
    uart_send(*str++);
  }
}
