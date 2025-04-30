#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>

void itoa_base(unsigned long long val, char *buf, int base, int width,
               int zero_pad, int uppercase) {
  const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
  char tmp[65];
  int i = 0;
  if (val == 0) {
    tmp[i++] = '0';
  } else {
    while (val > 0) {
      tmp[i++] = digits[val % base];
      val /= base;
    }
  }

  // padd if extra space is needed
  if (width != 0) {
    int pad = width - i;
    if (pad > 0) {
      for (int j = 0; j < pad; j++) {
        tmp[j] = zero_pad ? '0' : ' ';
      }
      i += pad;
    }
  }

  // reverse the string
  int j = 0;
  while (i > 0) {
    buf[j++] = tmp[--i];
  }
  buf[j] = '\0';
}

void itoa_dec(int val, char *buf) {
  if (val < 0) {
    *buf++ = '-';
    val = -val;
  }
  itoa_base(val, buf, 10, 0, 0, 0);
}

void itoa_hex(int val, char *buf) { itoa_base(val, buf, 16, 0, 0, 1); }

// strlen
size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len])
    len++;
  return len;
}

#endif //__UTILS_H__
