#include "uart.h"

void uart_init() {}

void uart_send(char c) {}

char uart_recv() { return 'a'; }

void uart_send_string(char *str) {
  while (*str) {
    uart_send(*str++);
  }
}
