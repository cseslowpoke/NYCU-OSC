#include "core/fork.h"
#include "common/types.h"
#include "core/sched.h"
#include "core/task.h"
#include "mm/slab.h"

static __attribute__((naked)) void return_from_fork() {
  __asm__ volatile("ldr x0, [sp, #0]\n\t"
                   "msr elr_el1, x0\n\t" // set elr_el1 to return value
                   "ldr x0, [sp, #8]\n\t"
                   "msr spsr_el1, x0\n\t" // set spsr_el1 to return value
                   "ldr x0, [sp, #16]\n\t"
                   "msr sp_el0, x0\n\t" // set sp_el0 to return value
                   "ldp x0, x1, [sp, #24]\n\t"
                   "ldp x2, x3, [sp, #(3 * 8) + 16 * 1]\n\t"
                   "ldp x4, x5, [sp, #(3 * 8) + 16 * 2]\n\t"
                   "ldp x6, x7, [sp, #(3 * 8) + 16 * 3]\n\t"
                   "ldp x8, x9, [sp, #(3 * 8) + 16 * 4]\n\t"
                   "ldp x10, x11, [sp, #(3 * 8) + 16 * 5]\n\t"
                   "ldp x12, x13, [sp, #(3 * 8) + 16 * 6]\n\t"
                   "ldp x14, x15, [sp, #(3 * 8) + 16 * 7]\n\t"
                   "ldp x16, x17, [sp, #(3 * 8) + 16 * 8]\n\t"
                   "ldp x18, x19, [sp, #(3 * 8) + 16 * 9]\n\t"
                   "ldp x20, x21, [sp, #(3 * 8) + 16 * 10]\n\t"
                   "ldp x22, x23, [sp, #(3 * 8) + 16 * 11]\n\t"
                   "ldp x24, x25, [sp, #(3 * 8) + 16 * 12]\n\t"
                   "ldp x26, x27, [sp, #(3 * 8) + 16 * 13]\n\t"
                   "ldp x28, x29, [sp, #(3 * 8) + 16 * 14]\n\t"
                   "ldr x30, [sp, #(3 * 8) + 16 * 15]\n\t"

                   "add sp, sp, 34 * 8\n\t" // pop 16 * 8 = 128 bytes
                   "eret\n\t");
}

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
  child->context.lr = (uint64_t)return_from_fork;

  // Add child to the scheduler
  sched_add(child);

  return child->pid;
}
