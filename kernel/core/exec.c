#include "core/exec.h"
#include "common/list.h"
#include "common/printf.h"
#include "common/types.h"
#include "common/utils.h"
#include "core/sched.h"
#include "core/task.h"
#include "drivers/irq.h"
#include "fs/file.h"
#include "mm/mm.h"
#include "mm/mmu.h"
#include "mm/slab.h"

void user_exec(void *prog, uint32_t prog_len) {
  uint64_t *prog_ptr = (uint64_t *)prog;
  uint64_t program_addr = USER_SPACE_BEGIN;
  for (uint32_t i = 0; i < prog_len / 8; i++) {
    ((uint64_t *)program_addr)[i] = prog_ptr[i];
  }
  program_addr = USER_SPACE_BEGIN;

  uint64_t stack_begin = USER_STACK_END;
  asm volatile("mov x0, %0\n\t"
               "msr sp_el0, x0\n\t"
               :
               : "r"(stack_begin));

  asm volatile("mov x0, %0\n\t"
               "msr elr_el1, x0\n\t"
               :
               : "r"(program_addr));

  asm volatile("mov x0, #0x340\n\t"
               "msr spsr_el1, x0\n\t");

  asm volatile("eret");
}

int32_t do_exec(const char *filename, char *const argv[]) {
  DISABLE_IRQ();
  void *user_prog;
  int32_t prog_len = file_find(filename, (char **)&user_prog);
  if (prog_len == -1) {
    printf("File not found: %s\r\n", filename);
    return -1;
  }

  // 1. Get the current task
  task_struct_t *task = get_current();

  // 2. Free old page table;

  // 3. Allocate new program and user stack
  task->prog = user_prog;

  // 4. Reset the task type
  task->type = TASK_USER;

  // 5. Set up the task context
  trapframe_t *trapframe =
      (trapframe_t *)((uint8_t *)task->stack + KERNEL_STACK_SIZE -
                      sizeof(trapframe_t));
  task->trapframe = trapframe;
  task->context.sp = (uint64_t)task->trapframe;
  task->context.lr = (uint64_t)task_return_el0;
  task->irq_priority = 0x3f3f3f3f;

  memset(trapframe, 0, sizeof(trapframe_t));
  trapframe->elr_el1 = 0;
  trapframe->spsr_el1 = 0x340;        // Set the SPSR to EL1h
  trapframe->sp_el0 = USER_STACK_END; // Set the stack pointer for EL0

  // 6. setup page table
  task->pgd = mmu_create_pg();
  INIT_LIST_HEAD(&task->vm_area_list);

  vm_area_t *prog_vma = kmalloc(sizeof(vm_area_t));
  prog_vma->start = USER_SPACE_BEGIN;
  prog_vma->end = round_up(USER_SPACE_BEGIN + prog_len, PAGE_SIZE);
  prog_vma->prot = MAIR_NORMAL | AP_RO_EL0 | PD_ACCESS;
  prog_vma->file = user_prog;
  INIT_LIST_HEAD(&prog_vma->list);
  vma_insert(task, prog_vma);

  vm_area_t *stack_vma = kmalloc(sizeof(vm_area_t));
  stack_vma->start = USER_STACK_BEGIN;
  stack_vma->end = USER_STACK_END;
  stack_vma->prot = MAIR_NORMAL | AP_RW_EL0 | PD_ACCESS;
  stack_vma->file = NULL;
  INIT_LIST_HEAD(&stack_vma->list);
  vma_insert(task, stack_vma);

  // map peripheral memory
  vm_area_t *peripheral_vma = kmalloc(sizeof(vm_area_t));
  peripheral_vma->start = PERIPHERAL_BEGIN;
  peripheral_vma->end = PERIPHERAL_END;
  peripheral_vma->prot = MAIR_DEVICE | AP_RW_EL0 | PD_ACCESS;
  peripheral_vma->file = (void *)phy_to_virt(PERIPHERAL_BEGIN);
  INIT_LIST_HEAD(&peripheral_vma->list);
  mmu_map(task->pgd, PERIPHERAL_BEGIN, PERIPHERAL_BEGIN,
          PERIPHERAL_END - PERIPHERAL_BEGIN, peripheral_vma->prot);
  vma_insert(task, peripheral_vma);

  mmu_switch_to(task->pgd);

  // 7. Handle arguments (for now, we don't do anything with them)
  // if (argv != NULL) {
  //   int argc = 0;
  //   while (argv[argc] != NULL) {
  //     trapframe->gpr[argc] = (uint64_t)argv[argc];
  //     argc++;
  //   }
  // }

  // 8. reset signal
  memset(task->signal.handler, 0, sizeof(task->signal.handler));

  // 9. return to user space
  static task_struct_t dummy;
  switch_to(&dummy, task);

  while (1) // should never reach here
    ;
  return 0;
}
