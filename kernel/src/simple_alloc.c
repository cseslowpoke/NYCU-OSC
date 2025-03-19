#include "simple_alloc.h"
#include "types.h"

void *simple_alloc(unsigned int size) {
  if (heap_current + size >= heap_end) {
    return NULL;
  }
  volatile char *ret = heap_current;
  heap_current += size;
  heap_current = (char *)(((uint64_t)heap_current + 7) & ~7);
  return ret;
}

void simple_alloc_init() {
  heap_start = &_heap_start;
  heap_end = &_heap_start + 0x80000;
  heap_current = heap_start;
}
