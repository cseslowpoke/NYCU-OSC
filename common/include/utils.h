#ifndef __UTILS_H
#define __UTILS_H

#define NULL 0

void delay_cycles(int cycles);

void uint2hex(unsigned int val, char *buf);

unsigned int hex2uint(char *hex, int size);

int atoi(const char *str);

#endif // __UTILS_H
