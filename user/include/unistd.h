#ifndef __UNISTD_H__
#define __UNISTD_H__

#include "syscall.h"
#include <stddef.h>

// pid
static inline int get_pid(void) { return syscall(SYS_GETPID, 0, 0, 0); }

// fork
static inline int fork(void) { return syscall(SYS_FORK, 0, 0, 0); }

static inline size_t read(void *buf, size_t count) {
  return syscall(SYS_UART_READ, (uint64_t)buf, count, 0);
}

static inline size_t write(const void *buf, size_t count) {
  return syscall(SYS_UART_WRITE, (uint64_t)buf, count, 0);
}

// exit
static inline void exit(void) {
  syscall(SYS_EXIT, 0, 0, 0);
  while (1)
    ;
}

#endif
