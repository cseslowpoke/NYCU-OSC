#include "core/mm.h"
#include "common/list.h"
#include "common/types.h"
#include "core/simple_alloc.h"

#include "common/printf.h"

static list_head_t free_list[MAX_ORDER + 1];

static buddy_block_t buddy_meta[(MM_ZONE_END - MM_ZONE_BEGIN) / PAGE_SIZE];

void mm_init() {
  uint32_t max_order_size =
      (MM_ZONE_END - MM_ZONE_BEGIN) / (PAGE_SIZE << (MAX_ORDER));
  for (int i = 0; i <= MAX_ORDER; i++) {
    INIT_LIST_HEAD(&free_list[i]);
  }
  for (int i = 0; i < max_order_size; i++) {
    buddy_meta[i << MAX_ORDER].id = i << MAX_ORDER;
    printf("[mm] id: %x\r\n", buddy_meta[i << MAX_ORDER].id);
    buddy_meta[i << MAX_ORDER].order = MAX_ORDER;
    buddy_meta[i << MAX_ORDER].used = 0;
    list_add_tail(&buddy_meta[i << MAX_ORDER].list, &free_list[MAX_ORDER]);
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

  buddy_block_t *block = list_entry(free_list[i].next, buddy_block_t, list);
  list_del(&block->list);
  printf("[mm] block_id: %x\r\n", block->id);

  for (int j = i; j > offer; j--) {

    // find buddy block
    int buddy_id = block->id ^ (1 << (j - 1));
    printf("[mm] buddy_id: %x\r\n", buddy_id);
    buddy_block_t *buddy = &buddy_meta[buddy_id];
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
  buddy_block_t *block = &buddy_meta[id];
  block->used = 0;

  printf("[mm] free: 0x%p, order: %d, id: %x\r\n",
         (void *)(uint64_t)(block->id * PAGE_SIZE + MM_ZONE_BEGIN),
         block->order, block->id);

  int order = block->order;

  while (order < MAX_ORDER) {
    int buddy_id = block->id ^ (1 << order);
    buddy_block_t *buddy = &buddy_meta[buddy_id];

    if (buddy->used == 1) {
      break;
    }

    printf("[mm] buddy_id: %x\r\n", buddy_id);

    list_del(&buddy->list);
    block->id = block->id < buddy_id ? block->id : buddy_id;
    block->order++;
    order++;
  }
  list_add(&block->list, &free_list[order]);
}

void mm_reserve(uint64_t begin, uint64_t end) {
  if (begin < MM_ZONE_BEGIN || end > MM_ZONE_END) {
    return;
  }
  uint32_t start = (begin - MM_ZONE_BEGIN) / PAGE_SIZE;
  uint32_t size = (end - begin + PAGE_SIZE - 1) / PAGE_SIZE;
  for (uint32_t i = start; i < start + size; i++) {
    int block_id = i;
    buddy_meta[block_id].id = block_id;
    if (!list_empty(&buddy_meta[block_id].list)) {
      list_del(&buddy_meta[block_id].list);
    }
    buddy_meta[block_id].used = 1;
    for (int i = 0; i < MAX_ORDER; i++) {
      int buddy_id = block_id ^ (1 << i);
      buddy_meta[buddy_id].id = buddy_id;
      buddy_meta[buddy_id].order = i;
      buddy_block_t *buddy = &buddy_meta[buddy_id];
      // NOTE: add note
      if (buddy->used == 1) {
        continue;
      }
      list_add_tail(&buddy_meta[buddy_id].list, &free_list[i]);
      block_id = block_id < buddy_id ? block_id : buddy_id;
    }
  }
}
