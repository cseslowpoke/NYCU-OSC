
#include "uart.h"

#define KERNEL_BASE_ADDRESS ((volatile unsigned int *)(0x80000))

int main() {
  uart_init();
  uart_send_string("UART bootloader: Waiting for kernel...\r\n");

  volatile unsigned int *des = KERNEL_BASE_ADDRESS;

  return 0;
}
