#include "core/syscall.h"
#include "common/printf.h"
#include "common/types.h"
#include "core/exec.h"
#include "core/fork.h"
#include "core/sched.h"
#include "core/signal.h"
#include "core/task.h"
#include "drivers/irq.h"
#include "drivers/mailbox.h"
#include "drivers/uart.h"
#include "mm/slab.h"

typedef uint64_t (*syscall_handler_t)(uint64_t, uint64_t, uint64_t, uint64_t,
                                      uint64_t, uint64_t);
#define SYSCALL_DEF(num, name, ret_type, args) [num] = (void *)sys_##name,

static syscall_handler_t syscall_table[] = {
#include "core/syscall_table.h"
};

#undef SYSCALL_DEF

int32_t syscall_handler(trapframe_t *tf) {
  uint64_t syscall_num = tf->gpr[8]; // x8
  if (syscall_num > SYSCALL_MAX) {
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

int32_t sys_fork() { return do_fork(); }

void sys_exit(int status) {
  task_struct_t *current = get_current();
  current->state = TASK_ZOMBIE;
  while (1)
    ;
}

int32_t sys_mbox_call(uint8_t ch, uint32_t *mbox) {
  return mailbox_call_with_mail(ch, mbox);
}

void sys_pkill(int pid) {
  task_struct_t *current = get_current();
  if (pid == current->pid) {
    current->state = TASK_ZOMBIE;
    sched();
  } else {
    sched_kill_task(pid);
  }
}

void sys_signal(int signum, sig_handler_t handler) {
  signal_register(signum, handler);
}

void sys_kill(int pid, int signum) { signal_send(pid, signum); }

void sys_sigreturn() { signal_return(); }
