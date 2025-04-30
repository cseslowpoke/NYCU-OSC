#ifndef __STDIO_H__
#define __STDIO_H__

#include "syscall.h"
#include "unistd.h"
#include "utils.h"
#include <stdarg.h>
#include <stddef.h>

int vsprintf(char *buf, const char *fmt, va_list args) {
  char *str = buf;
  for (; *fmt; fmt++) {
    if (*fmt != '%') {
      *str++ = *fmt;
      continue;
    }
    fmt++;
    switch (*fmt) {
    case 'c':
      *str++ = (char)va_arg(args, int);
      break;
    case 's': {
      const char *s = va_arg(args, const char *);
      while (*s) {
        *str++ = *s++;
      }
      break;
    }
    case 'd': {
      const int d = va_arg(args, int);
      char itoa_buf[32];
      itoa_dec(d, itoa_buf);
      char *itoa_ptr = itoa_buf;
      while (*itoa_ptr) {
        *str++ = *itoa_ptr++;
      }

      break;
    }
    case 'x': {
      const int d = va_arg(args, int);
      char itoa_buf[32];
      itoa_hex(d, itoa_buf);
      char *itoa_ptr = itoa_buf;
      while (*itoa_ptr) {
        *str++ = *itoa_ptr++;
      }
      break;
    }
    case 'p': {
      const void *p = va_arg(args, void *);
      char itoa_buf[32];
      itoa_base((uint64_t)p, itoa_buf, 16, 0, 0, 1);
      char *itoa_ptr = itoa_buf;
      while (*itoa_ptr) {
        *str++ = *itoa_ptr++;
      }
      break;
    }
    // NOTE: can add more format specifiers here
    default:
      *str++ = '%';
      *str++ = *fmt;
      break;
    }
  }
  *str = '\0';
  return str - buf;
}
void printf(const char *fmt, ...) {
  char buf[512];
  va_list args;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  write(buf, strlen(buf));
}

#endif
