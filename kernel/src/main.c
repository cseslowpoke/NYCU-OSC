#include "cpio.h"
#include "fdt.h"
#include "shell.h"
#include "simple_alloc.h"
#include "uart.h"
#include "utils.h"
int main(void *dtb_addr) {
  uart_init();
  simple_alloc_init();
  fdt_init(dtb_addr);
  cpio_init();

  shell_start();
  return 0;
}
