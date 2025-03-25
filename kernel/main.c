#include "core/shell.h"
#include "core/simple_alloc.h"
#include "drivers/irq.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "fs/cpio.h"
#include "fs/fdt.h"

int main(void *dtb_addr) {
  irq_init();
  uart_init();
  simple_alloc_init();
  fdt_init(dtb_addr);
  cpio_init();
  timer_init();
  shell_start();
  return 0;
}
