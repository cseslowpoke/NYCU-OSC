#ifndef __SLAB_H
#define __SLAB_H
/*
 * Slab Allocator
 *  ------
 * | buf |
 * ------
 */

#include "common/list.h"
#include "common/types.h"

#define KMEM_SIZE_CLASS 10

typedef struct kmem_cache {
  uint32_t slab_order;
  list_head_t free_list;
  uint32_t free_list_size;
  list_head_t partial_list;
  list_head_t full_list;
} kmem_cache_t;

typedef struct kmem_slab {
  uint32_t class;
  uint32_t inuse;
  uint32_t total;
  list_head_t free_list;
  list_head_t next;
} kmem_slab_t;

void kmem_cache_init(void);

void *kmem_cache_alloc(uint32_t class);

void kmem_cache_free(void *ptr);

void *kmalloc(uint32_t size);

void kfree(void *ptr);

#endif // __SLAB_H
