#ifndef __DELAY_H__
#define __DELAY_H__

static inline void delay(unsigned long ticks) {
  while (ticks--) {
    asm volatile("nop");
  }
}

#endif
