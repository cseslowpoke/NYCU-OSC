#include "cpio.h"
#include "shell.h"
#include "uart.h"
int main() {
  uart_init();
  cpio_init();
  shell_start();
  return 0;
}
