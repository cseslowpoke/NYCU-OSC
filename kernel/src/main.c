#include "cpio.h"
#include "shell.h"
#include "simple_alloc.h"
#include "uart.h"
int main() {
  uart_init();
  cpio_init();
  simple_alloc_init();

  shell_start();
  return 0;
}
