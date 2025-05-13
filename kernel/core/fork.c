#include "core/fork.h"
#include "common/types.h"
#include "core/exec.h"
#include "core/sched.h"
#include "core/task.h"
#include "mm/mmu.h"
#include "mm/slab.h"
#include "core/syscall.h"

struct signal_info_t;

int32_t do_fork() {
  task_struct_t *current = get_current();
  if (!current) {
    return -1;
  }

  task_struct_t *child = task_create_user();
  if (!child) {
    return -1;
  }

  child->prog = kmalloc(current->prog_size);
  if (!child->prog) {
    // TODO: destory task by function
    kfree(child->stack);
    kfree(child->user_stack);
    kfree(child->prog);
    kfree(child);
    return -1;
  }

  child->prog_size = current->prog_size;
  memcpy(child->prog, current->prog, current->prog_size);

  // copy current stack to child
  memcpy(child->user_stack, current->user_stack, USER_STACK_SIZE);

  child->state = TASK_SLEEPING;
  child->trapframe = child->stack + KERNEL_STACK_SIZE -
                     sizeof(trapframe_t); // set trapframe to child stack
  memcpy(child->trapframe, current->trapframe, sizeof(trapframe_t));

  child->trapframe->gpr[0] = 0;

  child->pgd = mmu_create_pg();
  mmu_map(child->pgd, USER_SPACE_BEGIN, (uint64_t)virt_to_phy(child->prog),
          current->prog_size,  MAIR_NORMAL | PD_ACCESS | AP_RW_EL0);
  mmu_map(child->pgd, USER_STACK_BEGIN, (uint64_t)virt_to_phy(child->user_stack),
          USER_STACK_SIZE, MAIR_NORMAL | PD_ACCESS | AP_RW_EL0);

  mmu_map(child->pgd, PERIPHERAL_BEGIN, PERIPHERAL_BEGIN,
          PERIPHERAL_END - PERIPHERAL_BEGIN,
          MAIR_DEVICE | AP_RW_EL0 | PD_ACCESS);

  // Set up context for the child
  child->context.sp = (uint64_t)child->trapframe;
  child->context.fp = (uint64_t)child->trapframe;
  child->context.lr = (uint64_t)task_return_el0;

  // copy parent signal to child
  memcpy(&child->signal, &current->signal,
         sizeof(signal_info_t)); // copy signal struct

  // Add child to the scheduler
  sched_add(child);

  return child->pid;
}
