#ifndef __MM_H
#define __MM_H

#include "common/list.h"
#include "common/types.h"
#include "core/slab.h"

#define MIN_ORDER 12
#define MAX_ORDER 10
#define PAGE_SIZE 0x1000

#define MM_ZONE_BEGIN 0x0
#define MM_ZONE_END 0x3C000000

typedef struct page {
  struct list_head list; // List head for free pages
  uint32_t id;           // ID of the block
  uint8_t order;         // Order of the block
  uint8_t used;          // Used flag

  kmem_slab_t *slab; // Pointer to the slab of this page
} page_t;

extern page_t *page_meta;

#define addr_to_page(addr)                                                     \
  (&page_meta[(((uint64_t)(addr) - MM_ZONE_BEGIN) / PAGE_SIZE)])

void mm_init(void);

void *mm_alloc(uint32_t size);

void mm_free(void *ptr);

#define MAX_RESERVED_REGIONS 64

typedef struct {
  uint64_t begin;
  uint64_t end;
} reserved_region_t;

static reserved_region_t reserved_regions[MAX_RESERVED_REGIONS];
static int reserved_count = 0;

void mm_reserve_region(uint64_t begin, uint64_t end);

#endif // __MM_H
