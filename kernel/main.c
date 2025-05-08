#include "common/printf.h"
#include "core/sched.h"
#include "core/shell.h"
#include "core/task.h"
#include "drivers/irq.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "fs/cpio.h"
#include "fs/fdt.h"
#include "mm/mm.h"
#include "mm/simple_alloc.h"
#include "mm/slab.h"
#include "common/utils.h"

extern char __kernel_start[];
extern char __kernel_end[];

int main(void *dtb_addr) {
  __asm__ volatile(
    "ldr x1, =0\n\t"
    "ldr x0, =0x3000\n\t"
    "str x1, [x0]\n\t"
    "msr ttbr0_el1, x0\n\t"
  );
  mm_reserve_region((uint64_t)__kernel_start, (uint64_t)__kernel_end);
  mm_reserve_region(VM_BASE + 0x0, VM_BASE + 0x1000);
  mm_reserve_region(VM_BASE + 0x1000, VM_BASE + 0x3000);
  irq_init();
  uart_init();
  simple_alloc_init();
  fdt_init(dtb_addr);
  cpio_init();
  mm_init();
  kmem_cache_init();
  timer_init();

  sched_init();

  task_test();

  // shell_start();
  return 0;
}
