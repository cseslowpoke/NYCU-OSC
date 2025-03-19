#include "common/string.h"
#include "common/types.h"

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

char *strtok(char *str, const char delim) {
  // static unsigned int sz1 = NULL;
  static char *s1 = NULL;
  static char *s2 = NULL;
  if (str != NULL) {
    strip(&str, delim);
    s1 = NULL;
    s2 = str;
    for (; *str != '\0'; str++) {
      if (*str == delim) {
        *str = '\0';
        s1 = str + 1;
        break;
      }
    }
  } else {
    strip(&s1, delim);
    s2 = s1;
    for (; *s1 != '\0'; s1++) {
      if (*s1 == delim) {
        *s1 = '\0';
        s1 = s1 + 1;
        break;
      }
    }
  }
  if (*s2 == '\0') {
    return NULL;
  }
  return s2;
}

void strip(char **str, const char delim) {
  while (**str == delim) {
    (*str)++;
  }
}

unsigned int strlen(const char *s) {
  unsigned int len = 0;
  while (*s++) {
    len++;
  }
  return len;
}

char *strcpy(char *dest, const char *src) {
  char *ret = dest;
  while (*src) {
    *dest++ = *src++;
  }
  *dest = '\0';
  return ret;
}
