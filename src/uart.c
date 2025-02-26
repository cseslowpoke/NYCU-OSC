#include "uart.h"
#include "utils.h"

void uart_init() {
  // setting gpio 14 and 15 to alternative function 5 (mini uart)
  volatile int *GPIO_FUNC_SEL = (int *)(PBASE | GPFSEL1);
  *GPIO_FUNC_SEL = 0x12000;

  // setting gpio Pull-up/down control register
  volatile int *GPIO_PUD = (int *)(PBASE | GPPUD);
  volatile int *GPIO_PUD_CLK = (int *)(PBASE | GPPUDCLK0);
  *GPIO_PUD = 0x1;
  delay_cycles(150);
  *GPIO_PUD_CLK = 1 << 15;
  delay_cycles(150);
  *GPIO_PUD = 0;
  *GPIO_PUD_CLK = 0;
  return;
}

void uart_send(char c) {}

char uart_recv() { return 'a'; }

void uart_send_string(char *str) {
  while (*str) {
    uart_send(*str++);
  }
}
