#include "core/signal.h"
#include "common/printf.h"
#include "core/sched.h"
#include "core/task.h"
#include "mm/slab.h"

void signal_register(int signum, sig_handler_t handler) {
  task_struct_t *task = get_current();
  if (0 <= signum && signum < SIGN) {
    task->signal.handler[signum] = handler;
  }
}

void signal_send(int pid, int signum) {
  task_struct_t *task = sched_get_task_by_pid(pid);
  if (task) {
    task->signal.pending |= (1 << signum);
  }
}

void default_signal_handler(task_struct_t *task, int signum) {
  switch (signum) {
  case 9: // SIGKILL
    task->state = TASK_ZOMBIE;
    sched();
    break;
  default:
    break;
  }
}

__attribute__((naked)) void __sigreturn_trampoline() {
  asm volatile("mov x8, #10\n\t" // syscall number for sigreturn
               "svc #0\n\t"      // syscall
               "b .\n\t");
}

void signal_invoke_handler(task_struct_t *task, sig_handler_t handler) {

  // allocate a new stack for the signal handler
  task->signal.stack = kmalloc(TASK_STACK_SIZE);
  if (!task->signal.stack) {
    printf("Failed to allocate memory for signal stack\n");
    return; // Handle memory allocation failure
  }
  // save the current trapframe to the signal stack
  trapframe_t *sig_tf = (trapframe_t *)((uint8_t *)task->signal.stack +
                                        TASK_STACK_SIZE - sizeof(trapframe_t));
  memcpy(sig_tf, task->trapframe,
         sizeof(trapframe_t)); // Copy the current trapframe to the new stack

  trapframe_t *tf = task->trapframe;
  tf->gpr[0] = (uint64_t)NULL;
  tf->gpr[30] = (uint64_t)__sigreturn_trampoline;
  tf->elr_el1 = (uint64_t)handler;
  tf->sp_el0 = (uint64_t)sig_tf;

  task->context.sp = (uint64_t)tf;
  task->context.lr = (uint64_t)task_return_el0;
  task->irq_priority = 0x3f3f3f3f;

  task_struct_t dummy;
  switch_to(&dummy, task); // Switch to the signal handler
}

void signal_handler() {
  task_struct_t *task = get_current();
  uint32_t pending = task->signal.pending;
  for (int i = 0; i < SIGN; i++) {
    if (pending & (1 << i)) {
      task->signal.pending &= ~(1 << i);
      sig_handler_t handler = task->signal.handler[i];
      if (handler) {
        signal_invoke_handler(task, handler);
      } else {
        default_signal_handler(task, i);
      }
    }
  }
}

void signal_return() {
  task_struct_t *task = get_current();
  trapframe_t *tf = (trapframe_t *)task->trapframe->sp_el0;
  memcpy(task->trapframe, tf, sizeof(trapframe_t));
  kfree(task->signal.stack);
  task->signal.stack = NULL;
}
