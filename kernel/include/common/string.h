#ifndef __STRING_H
#define __STRING_H

/*
 * strcmp - compare two string and return the difference in the first different
 * character.
 * @param s1 - first string.
 * @param s2 - second string.
 */
int strcmp(const char *s1, const char *s2);

/*
 * strtok - split a string into tokens.
 * @param str - string to split.
 * @param delim - delimiter.
 *
 * example:
 * e.g. strtok("hello world", ' ') will return "hello", strtok(NULL, ' ') will
 * return "world".
 */
char *strtok(char *str, const char delim);

/*
 * strip - eliminate leading delimiters.
 * @param str - string to strip.
 * @param delim - delimiter.
 */
void strip(char **str, const char delim);

/*
 * strlen - return the length of a string.
 * @param s - string.
 */
unsigned int strlen(const char *s);

/*
 * strcpy - copy a string from src to dest.
 * @param dest - destination string.
 * @param src - source string.
 */
char *strcpy(char *dest, const char *src);

#endif
