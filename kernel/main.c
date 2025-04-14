#include "common/printf.h"
#include "core/mm.h"
#include "core/shell.h"
#include "core/simple_alloc.h"
#include "core/slab.h"
#include "drivers/irq.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "fs/cpio.h"
#include "fs/fdt.h"

extern char __kernel_start[];
extern char __kernel_end[];

int main(void *dtb_addr) {
  mm_reserve_region((uint64_t)__kernel_start, (uint64_t)__kernel_end);
  mm_reserve_region(0x0, 0x1000);
  simple_alloc_init();
  fdt_init(dtb_addr);
  cpio_init();
  irq_init();
  uart_init();
  mm_init();
  kmem_cache_init();
  timer_init();
  void *a[1000];
  for (int i = 0; i < 100; i++) {
    a[i] = kmalloc(0x400000);
    // printf("kmalloc: 0x%p\r\n", a);
  }
  for (int i = 0; i < 100; i++) {
    kfree(a[i]);
    // printf("kfree: 0x%p\r\n", a);
  }

  // timer_lab3_basic2_task();
  shell_start();
  return 0;
}
