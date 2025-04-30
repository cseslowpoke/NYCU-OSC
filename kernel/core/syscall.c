#include "core/syscall.h"
#include "common/printf.h"
#include "common/types.h"
#include "core/sched.h"
#include "core/task.h"
#include "core/user_exec.h"
#include "drivers/irq.h"
#include "drivers/mailbox.h"
#include "drivers/uart.h"
#include "mm/slab.h"

typedef uint64_t (*syscall_handler_t)(uint64_t, uint64_t, uint64_t, uint64_t,
                                      uint64_t, uint64_t);
#define SYSCALL_DEF(num, name, ret_type, args) [num] = (void *)sys_##name,

syscall_handler_t syscall_table[] = {
#include "core/syscall_table.h"
};

#undef SYSCALL_DEF

int32_t syscall_handler(trapframe_t *tf) {
  uint64_t syscall_num = tf->gpr[8]; // x8
  if (syscall_num >= SYSCALL_MAX) {
    return -1;
  }
  syscall_handler_t syscall_fn = syscall_table[syscall_num];
  if (syscall_fn == NULL) {
    return -1;
  }
  tf->gpr[0] = syscall_fn(tf->gpr[0], tf->gpr[1], tf->gpr[2], tf->gpr[3],
                          tf->gpr[4], tf->gpr[5]);
  return 0;
}

int32_t sys_getpid() {
  task_struct_t *current = get_current();
  return current->pid;
}

uint32_t sys_uart_read(char buf[], uint32_t size) {
  ENABLE_IRQ();
  for (int i = 0; i < size; i++) {
    buf[i] = uart_recv();
  }
  DISABLE_IRQ();
  return size;
}

uint32_t sys_uart_write(const char buf[], uint32_t size) {
  for (int i = 0; i < size; i++) {
    uart_send(buf[i]);
  }
  return size;
}

int32_t sys_exec(const char *name, char *const argv[]) {
  return do_exec(name, argv);
}

__attribute__((naked)) void return_from_fork() {
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

int32_t sys_fork() {
  task_struct_t *current = get_current();
  task_struct_t *child = task_create_user();

  child->prog = kmalloc(current->prog_size);
  child->prog_size = current->prog_size;
  memcpy(child->prog, current->prog, current->prog_size);

  // copy current stack to child
  memcpy(child->user_stack, current->user_stack, TASK_STACK_SIZE);
  child->state = TASK_SLEEPING;
  child->trapframe = child->stack + TASK_STACK_SIZE -
                     sizeof(trapframe_t); // set trapframe to child stack
  memcpy(child->trapframe, current->trapframe, sizeof(trapframe_t));
  child->trapframe->gpr[0] = 0; // child process return 0

  child->trapframe->gpr[29] =
      current->trapframe->gpr[29] - (uint64_t)current->user_stack +
      (uint64_t)child->user_stack; // set fp to child stack
  child->trapframe->gpr[30] = current->trapframe->gpr[30] -
                              (uint64_t)current->prog +
                              (uint64_t)child->prog; // set lr to child stack
  child->trapframe->sp_el0 =
      current->trapframe->sp_el0 - (uint64_t)current->user_stack +
      (uint64_t)child->user_stack; // set sp_el0 to child stack
  child->trapframe->elr_el1 = current->trapframe->elr_el1 -
                              (uint64_t)current->prog +
                              (uint64_t)child->prog; // set elr_el1 to child

  child->context.sp = (uint64_t)child->trapframe;
  child->context.fp = (uint64_t)child->trapframe;
  child->context.lr = (uint64_t)return_from_fork;
  sched_add(child);
  return child->pid;
}

void sys_exit(int status) {
  task_struct_t *current = get_current();
  current->state = TASK_ZOMBIE;
  while (1)
    ;
}

int32_t sys_mbox_call(uint8_t ch, uint32_t *mbox) {
  return mailbox_call_with_mail(ch, mbox);
}

void sys_kill(int pid) {
  task_struct_t *current = get_current();
  if (pid == current->pid) {
    current->state = TASK_ZOMBIE;
    sched();
  } else {
    sched_kill_task(pid);
  }
}
