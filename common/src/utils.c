#include "utils.h"

void delay_cycles(int cycles) {
  while (cycles-- > 0) {
    __asm__ volatile("nop");
  }
}

void uint2hex(unsigned int val, char *buf) {
  for (int i = 0; i < 8; i++) {
    int nibble = (val >> (28 - i * 4)) & 0xF;
    if (nibble < 10) {
      buf[i] = '0' + nibble;
    } else {
      buf[i] = 'A' + nibble - 10;
    }
  }
  buf[8] = '\0';
}

unsigned int hex2uint(char *hex, int size) {
  unsigned int ret = 0;
  for (int i = 0; i < size; i++) {
    ret *= 16;
    if ('A' <= hex[i] && hex[i] <= 'F') {
      ret += 10 + hex[i] - 'A';
    } else if ('0' <= hex[i] && hex[i] <= '9') {
      ret += hex[i] - '0';
    }
  }
  return ret;
}
