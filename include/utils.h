static inline void delay_cycles(int cycles) {
  while (cycles-- > 0) {
    __asm__ volatile("nop");
  }
}
