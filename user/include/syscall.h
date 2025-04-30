#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <stdint.h>

// syscall number
enum {
  SYS_GETPID = 0,
  SYS_UART_READ,
  SYS_UART_WRITE,
  SYS_EXEC,
  SYS_FORK,
  SYS_EXIT,
  SYS_MBOX_CALL,
  SYS_KILL,
};

// user mode syscall
static inline uint64_t syscall(uint64_t num, uint64_t arg0, uint64_t arg1,
                               uint64_t arg2) {
  uint64_t ret;
  __asm__ volatile("mov x8, %1\n\t" // syscall number
                   "mov x0, %2\n\t" // arg0
                   "mov x1, %3\n\t" // arg1
                   "mov x2, %4\n\t" // arg2
                   "svc #0\n\t"     // syscall
                   "mov %0, x0\n\t" // return value
                   : "=r"(ret)
                   : "r"(num), "r"(arg0), "r"(arg1), "r"(arg2)
                   : "x0", "x1", "x2", "x8");
  return ret;
}

#endif
