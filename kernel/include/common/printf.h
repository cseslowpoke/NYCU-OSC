#ifndef __PRINTF_H
#define __PRINTF_H

#include <stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);

void printf(const char *fmt, ...);

void debug_printf(const char *fmt, ...);

#endif // __PRINTF_H
