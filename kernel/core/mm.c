// clang-format off
/*
 * Memory Management - Buddy Allocator
 * -----------------------------------
 * A power-of-two memory allocator for page-based memory management.
 *
 * Core API:
 * - void mm_init(): Initialize the memory management system.
 * - void *mm_alloc(uint32_t size): Allocate memory of the specified size.
 * - void mm_free(void *ptr): Free the allocated memory.
 * - void mm_reserve(uint64_t begin, uint64_t end): Reserve a range of memory.
 *   Procedure:
 *   - Start from nearest page of begin address.
 *   - if this page is in free list, remove it from free list.
 *   - set page as used.
 *   - Recursively move upward through higher order.
 *     - At each level, try to move buddy block to the free list.
 *     - choose next block with smaller id.
 *     - stop when level is MAX_ORDER.
 *
 *   Graph Example:
 *     Assume we has 8 pages of memory, and we want to reserve [4]
 *     Recursively find which page should add to free list.
 *     and remove root from free_list[MAX_ORDER].
 *      +-------------------------------+
 *      |               x               | <- 4. [0] should be remove from free_list[3].
 *      +---------------+---------------+
 *      |       o       |       x       | <- 3. [0] should add to free_list[2].
 *      +-------+-------+-------+-------+
 *      |       |       |   x   |   o   | <- 2. [6] should add to free_list[1].
 *      +---+---+---+---+---+---+---+---+
 *      |   |   |   |   | x | o |   |   | <- 1. Reserve at x [4], [5] should add to free_list[0].
 *      +---+---+---+---+---+---+---+---+
 *
 *     Complexity: O(log(n)).
 */
// clang-format on

#include "core/mm.h"
#include "common/list.h"
#include "common/printf.h"
#include "common/types.h"
#include "core/simple_alloc.h"

static list_head_t free_list[MAX_ORDER + 1];

page_t *page_meta;

void mm_init() {
  page_meta = (page_t *)simple_alloc((MM_ZONE_END - MM_ZONE_BEGIN) / PAGE_SIZE *
                                     sizeof(page_t));
  mm_reserve_region((uint64_t)simple_alloc_begin(),
                    (uint64_t)simple_alloc_end()); // reserve the whole memory

  uint32_t max_order_size =
      (MM_ZONE_END - MM_ZONE_BEGIN) / (PAGE_SIZE << (MAX_ORDER));
  int page_size = (MM_ZONE_END - MM_ZONE_BEGIN) / PAGE_SIZE;
  for (int i = 0; i <= MAX_ORDER; i++) {
    INIT_LIST_HEAD(&free_list[i]);
  }
  for (int i = 0; i < page_size; i++) {
    page_meta[i].id = i;
    page_meta[i].order = -1;
    INIT_LIST_HEAD(&page_meta[i].list);
  }
  // printf("%d", max_order_size);
  for (int i = 0; i < max_order_size; i++) {
    page_meta[i << MAX_ORDER].order = MAX_ORDER;
    page_meta[i << MAX_ORDER].used = 0;
    printf("[mm] block id = 0x%x\r\n", i << MAX_ORDER);
    list_add_tail(&page_meta[i << MAX_ORDER].list, &free_list[MAX_ORDER]);
  }

  uint8_t max_order_bit_map[max_order_size];
  for (int i = 0; i < max_order_size; i++) {
    max_order_bit_map[i] = 0;
  }

  for (int i = 0; i < reserved_count; i++) {
    uint64_t begin = reserved_regions[i].begin;
    uint64_t end = reserved_regions[i].end;
    // Find the nearest page of begin address and end address
    uint32_t start_page = (begin - MM_ZONE_BEGIN) / PAGE_SIZE;
    uint32_t end_page = (end - 1 - MM_ZONE_BEGIN) / PAGE_SIZE;
    for (uint32_t i = start_page; i <= end_page; i++) {
      page_t *block = &page_meta[i];
      block->used = 1;
    }
    uint32_t start_page_buddy = start_page & ~((1 << MAX_ORDER) - 1);
    uint32_t end_page_buddy = end_page & ~((1 << MAX_ORDER) - 1);
    for (int i = start_page_buddy; i <= end_page_buddy; i += 1 << MAX_ORDER) {
      max_order_bit_map[i >> MAX_ORDER] = 1;
      list_del(&page_meta[i].list);
    }
  }
  for (int i = 0; i < max_order_size; i++) {
    if (max_order_bit_map[i] == 0) {
      continue;
    }
    for (int j = i << MAX_ORDER; j < ((i + 1) << MAX_ORDER); j++) {
      page_t *block = &page_meta[j];
      if (block->used == 1) {
        continue;
      }

      for (int k = 0; k <= MAX_ORDER; k++) {
        page_t *buddy = &page_meta[block->id ^ (1 << k)];
        if (buddy->used == 1) {
          list_add(&block->list, &free_list[k]);
          block->order = k;
          break;
        }
        if (buddy->order == k) {
          list_del(&buddy->list);
          block = block->id < buddy->id ? block : buddy;
        } else {
          list_add(&block->list, &free_list[k]);
          block->order = k;
          break;
        }
      }
    }
  }
}

void *mm_alloc(uint32_t size) {
  // check if size isn't power of 2
  int offer = 0;
  int power2 = 1;
  while (power2 < size) {
    power2 <<= 1;
    offer++;
  }
  offer -= MIN_ORDER;
  if (offer <= 0) {
    offer = 0;
  }
  if (offer > MAX_ORDER) {
    return NULL;
  }

  int i = offer;
  for (; i < MAX_ORDER; i++) {
    if (!list_empty(&free_list[i])) {
      break;
    }
  }

  if (list_empty(&free_list[i])) {
    return NULL;
  }
  printf("[mm] offer: %d, i: %d\r\n", offer, i);

  page_t *block = list_entry(free_list[i].next, page_t, list);
  list_del(&block->list);
  printf("[mm] block_id: %x\r\n", block->id);

  for (int j = i; j > offer; j--) {

    // find buddy block
    int buddy_id = block->id ^ (1 << (j - 1));
    printf("[mm] buddy_id: %x\r\n", buddy_id);
    page_t *buddy = &page_meta[buddy_id];
    buddy->id = buddy_id;
    buddy->order = j - 1;
    buddy->used = 0;

    list_add(&buddy->list, &free_list[j - 1]);
  }
  block->order = offer;
  block->used = 1;

  printf("[mm] alloc: 0x%p, order: %d, id: %x\r\n",
         (void *)(uint64_t)(block->id * PAGE_SIZE + MM_ZONE_BEGIN),
         block->order, block->id);

  return (void *)(uint64_t)(block->id * PAGE_SIZE + MM_ZONE_BEGIN);
}

void mm_free(void *ptr) {
  printf("[mm] free: 0x%p\r\n", ptr);
  uint32_t id = ((uint32_t)(uint64_t)ptr - MM_ZONE_BEGIN) / PAGE_SIZE;
  page_t *block = &page_meta[id];
  block->used = 0;

  printf("[mm] free: 0x%p, order: %d, id: %x\r\n",
         (void *)(uint64_t)(block->id * PAGE_SIZE + MM_ZONE_BEGIN),
         block->order, block->id);

  while (block->order < MAX_ORDER) {
    int buddy_id = block->id ^ (1 << block->order);
    page_t *buddy = &page_meta[buddy_id];

    if (buddy->used == 1) {
      break;
    }

    printf("[mm] buddy_id: %x\r\n", buddy_id);

    list_del(&buddy->list);
    block = block->id < buddy->id ? block : buddy;
    block->order++;
  }
  list_add(&block->list, &free_list[block->order]);
}

void mm_reserve_region(uint64_t begin, uint64_t end) {
  if (begin >= end) {
    return;
  }
  if (begin < MM_ZONE_BEGIN || end > MM_ZONE_END) {
    return;
  }
  if (reserved_count >= MAX_RESERVED_REGIONS) {
    return;
  }
  reserved_regions[reserved_count].begin = begin;
  reserved_regions[reserved_count].end = end;
  reserved_count++;
}
