#include "core/fork.h"
#include "common/types.h"
#include "core/sched.h"
#include "core/task.h"
#include "mm/slab.h"

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
  memcpy(child->user_stack, current->user_stack, TASK_STACK_SIZE);

  child->state = TASK_SLEEPING;
  child->trapframe = child->stack + TASK_STACK_SIZE -
                     sizeof(trapframe_t); // set trapframe to child stack
  memcpy(child->trapframe, current->trapframe, sizeof(trapframe_t));

  // Set up trapframe for the child
  child->trapframe->gpr[0] = 0; // child process return 0
  child->trapframe->gpr[29] = current->trapframe->gpr[29] -
                              (uint64_t)current->user_stack +
                              (uint64_t)child->user_stack;
  child->trapframe->sp_el0 = current->trapframe->sp_el0 -
                             (uint64_t)current->user_stack +
                             (uint64_t)child->user_stack;

  // NOTE: For now, we using same code with parent.
  child->trapframe->gpr[30] = current->trapframe->gpr[30];
  child->trapframe->elr_el1 = current->trapframe->elr_el1;
  // child->trapframe->gpr[30] = current->trapframe->gpr[30] -
  //                             (uint64_t)current->prog +
  //                             (uint64_t)child->prog;
  // child->trapframe->elr_el1 = current->trapframe->elr_el1 -
  //                             (uint64_t)current->prog +
  //                             (uint64_t)child->prog;

  // Set up context for the child
  child->context.sp = (uint64_t)child->trapframe;
  child->context.fp = (uint64_t)child->trapframe;
  child->context.lr = (uint64_t)task_return_el0;

  // Add child to the scheduler
  sched_add(child);

  return child->pid;
}
