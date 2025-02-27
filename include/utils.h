#ifndef __UTILS_H
#define __UTILS_H

static inline void delay_cycles(int cycles) {
  while (cycles-- > 0) {
    __asm__ volatile("nop");
  }
}

#endif // __UTILS_H
