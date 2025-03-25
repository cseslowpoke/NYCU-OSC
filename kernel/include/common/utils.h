#ifndef __UTILS_H
#define __UTILS_H

#include "common/types.h"

void delay_cycles(int cycles);

void uint2hex(uint32_t val, char *buf);

uint32_t hex2uint(char *hex, int size);

int atoi(const char *str);

#define READ_SYSREG(sysreg)                                                    \
  ({                                                                           \
    uint64_t val;                                                              \
    asm volatile("mrs %0, " #sysreg : "=r"(val));                              \
    val;                                                                       \
  })

#define WRITE_SYSREG(sysreg, val)                                              \
  asm volatile("msr " #sysreg ", %0" : : "r"(val));

#define offsetof(type, member) ((uint64_t)&((type *)0)->member)

#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr) - offsetof(type, member)))

#endif // __UTILS_H
