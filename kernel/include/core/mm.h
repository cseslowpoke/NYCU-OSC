#ifndef __MM_H
#define __MM_H

#include "common/list.h"
#include "common/types.h"

#define MIN_ORDER 12
#define MAX_ORDER 10
#define PAGE_SIZE 0x1000

#define MM_ZONE_BEGIN 0x0
#define MM_ZONE_END 0x3C000000

typedef struct buddy_block {
  struct list_head list; // List head for free pages
  uint32_t id;           // ID of the block
  uint8_t order;         // Order of the block
  uint8_t used;          // Used flag
} buddy_block_t;

void mm_init(void);

void *mm_alloc(uint32_t size);

void mm_free(void *ptr);

void mm_reserve(uint64_t begin, uint64_t end);

#endif // __MM_H
