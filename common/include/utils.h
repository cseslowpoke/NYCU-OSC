#ifndef __UTILS_H
#define __UTILS_H

#include "types.h"

void delay_cycles(int cycles);

void uint2hex(uint32_t val, char *buf);

uint32_t hex2uint(char *hex, int size);

int atoi(const char *str);

#endif // __UTILS_H
