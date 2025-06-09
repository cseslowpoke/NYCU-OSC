#include "common/string.h"
#include "common/types.h"
#include "mm/slab.h"

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

char *strtok(char *str, const char delim) {
  static char *s1 = NULL;
  static char *s2 = NULL;
  if (str != NULL) {
    strip(&str, delim);
    s1 = str;
    s2 = str;
    for (; *s1 != '\0'; s1++) {
      if (*s1 == delim) {
        *s1 = '\0';
        s1 = s1 + 1;
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

char *strdup(const char *s) {
  unsigned int len = strlen(s);
  char *new_str = (char *)kmalloc(len + 1); // +1 for null terminator
  memset(new_str, 0, len + 1); // Initialize memory to zero
  if (new_str == NULL) {
    return NULL; // Allocation failed
  }
  strcpy(new_str, s);
  return new_str;
}

char *strrchr(const char *string, int c) {
  const char *last = NULL;
  while (*string) {
    if (*string == c) {
      last = string;
    }
    string++;
  }
  return (char *)last; // Return the last occurrence or NULL if not found
}