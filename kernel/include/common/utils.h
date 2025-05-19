#ifndef __UTILS_H
#define __UTILS_H

#include "common/types.h"

#define VM_BASE 0xffff000000000000ull
#define MMIO_BASE (VM_BASE + 0x3f000000)

#define DELAY_CYCLES(cycles)                                                   \
  do {                                                                         \
    int _c = (cycles);                                                         \
    while (_c-- > 0) {                                                         \
      __asm__ volatile("nop");                                                 \
    }                                                                          \
  } while (0)

/*
 * itoa_base - convert an integer to a string in a specified base.
 * @param val - the integer value to convert.
 * @param buf - the buffer to store the converted string.
 * @param base - the base to convert to (2, 8, 10, 16).
 * @param width - the minimum width of the output string, except for 0.
 *                if width is 0, will use original number width.
 * @param zero_pad - if true, pad with zeros instead of spaces.
 * @param uppercase - if true, use uppercase letters for hex digits.
 */
void itoa_base(unsigned long long val, char *buf, int base, int width,
               int zero_pad, int uppercase);

/*
 * itoa_dec - convert a decimal integer to a string.
 *            A wrapper of itoa_base with base 10.
 * @param val - the integer value to convert.
 * @param buf - the buffer to store the converted string.
 */
void itoa_dec(int val, char *buf);

/*
 * itoa_hex - convert a hexadecimal integer to a string.
 *           A wrapper of itoa_base with base 16.
 * @param val - the integer value to convert.
 * @param buf - the buffer to store the converted string.
 */
void itoa_hex(int val, char *buf);

void uint2hex(uint32_t val, char *buf);

uint32_t hex2uint(char *hex, int size);

int atoi(const char *str);

#define READ_SYSREG(sysreg)                                                    \
  ({                                                                           \
    uint64_t val;                                                              \
    asm volatile("mrs %0, " #sysreg : "=r"(val));                              \
    val;                                                                       \
  })

#define WRITE_SYSREG(sysreg, val)                                              \
  asm volatile("msr " #sysreg ", %0" : : "r"(val));

/*
 * offsetof - get the offset of a member in a struct.
 * @param type - type of the struct.
 * @param member - name of the member in the struct.
 */
#define offsetof(type, member) ((uint64_t)&((type *)0)->member)

/*
 * container_of - get the container of a member in a struct.
 * @param ptr - pointer to the member.
 * @param type - type of the container.
 * @param member - name of the member in the container.
 */
#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr) - offsetof(type, member)))

void *memcpy(void *dest, const void *src, uint32_t size);

void *memset(void *dest, int c, uint32_t size);

uint64_t round_up(uint64_t num, uint64_t align);

uint64_t round_down(uint64_t num, uint64_t align);

#endif // __UTILS_H
