#include "fs/cpio.h"
#include "fs/fdt.h"
#include "core/shell.h"
#include "core/simple_alloc.h"
#include "drivers/uart.h"
#include "common/utils.h"

int main(void *dtb_addr) {
  uart_init();
  simple_alloc_init();
  fdt_init(dtb_addr);
  cpio_init();
  shell_start();
  return 0;
}
