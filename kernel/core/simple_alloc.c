#include "core/simple_alloc.h"
#include "common/types.h"

extern char __simple_alloc_start[];
extern char __simple_alloc_end[];

static char *simple_alloc_current;

void *simple_alloc(unsigned int size) {
  if (simple_alloc_current + size >= __simple_alloc_end) {
    return NULL;
  }
  volatile char *ret = simple_alloc_current;
  simple_alloc_current += size;
  simple_alloc_current = (char *)(((uint64_t)simple_alloc_current + 7) & ~7);
  return ret;
}

void simple_alloc_init() { simple_alloc_current = __simple_alloc_start; }

void *simple_alloc_begin() { return __simple_alloc_start; }
void *simple_alloc_end() { return simple_alloc_current; }
