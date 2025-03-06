#include "simple_alloc.h"
#include "utils.h"

void *simple_alloc(unsigned int size) {
  if (heap_current + size >= heap_end) {
    return NULL;
  }
  char *ret = heap_current;
  heap_current += size;
  return ret;
}

void simple_alloc_init() {
  heap_start = &_heap_start;
  heap_end = &_heap_start + 0x10000;
  heap_current = heap_start;
}
