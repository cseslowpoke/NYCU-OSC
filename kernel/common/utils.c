#include "common/utils.h"

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

int atoi(const char *str) {
  // negative number
  if (*str == '-') {
    return -atoi(str + 1);
  }
  int ret = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    ret = ret * 10 + str[i] - '0';
  }
  return ret;
}
