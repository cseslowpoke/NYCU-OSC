#include "common/printf.h"
#include "core/shell.h"
#include "drivers/irq.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "fs/cpio.h"
#include "fs/fdt.h"
#include "mm/mm.h"
#include "mm/simple_alloc.h"
#include "mm/slab.h"

extern char __kernel_start[];
extern char __kernel_end[];

int main(void *dtb_addr) {
  mm_reserve_region((uint64_t)__kernel_start, (uint64_t)__kernel_end);
  mm_reserve_region(0x0, 0x1000);
  irq_init();
  uart_init();
  simple_alloc_init();
  fdt_init(dtb_addr);
  cpio_init();
  mm_init();
  kmem_cache_init();
  timer_init();

  shell_start();
  return 0;
}
