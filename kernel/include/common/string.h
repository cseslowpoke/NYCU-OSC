#ifndef __STRING_H
#define __STRING_H

int strcmp(const char *s1, const char *s2);

char *strtok(char *str, const char delim);

void strip(char **str, const char delim);

unsigned int strlen(const char *s);

char *strcpy(char *dest, const char *src);

#endif
