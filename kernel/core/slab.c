/*
 * Memory Maanagement - Slab Allocator
 * -----------------------------------
 * Efficient memory allocation for small objects.
 *
 * Reference:
 *   J. Bonwick, "The Slab Allocator: An Object-Caching Kernel Memory
 *   Allocator",
 *   https://people.eecs.berkeley.edu/~kubitron/courses/cs194-24-S13/hand-outs/bonwick_slab.pdf
 */

#include "core/slab.h"
#include "common/list.h"
#include "common/printf.h"
#include "common/types.h"
#include "core/mm.h"

static const uint32_t kmem_size_classes[] = {16,  32,   64,   128,  256,
                                             512, 1024, 2048, 4096, 8192};

static kmem_cache_t kmem_caches[KMEM_SIZE_CLASS];

kmem_slab_t *kmem_alloc_slab_large(uint32_t class) {
  void *ptr = mm_alloc(PAGE_SIZE * 16);
  kmem_slab_t *slab = (kmem_slab_t *)kmalloc(sizeof(kmem_slab_t));
  INIT_LIST_HEAD(&slab->free_list);
  INIT_LIST_HEAD(&slab->list);
  slab->slab_base = ptr;
  slab->inuse = 0;
  slab->total = 0;
  slab->class = class;
  for (uint64_t i = (uint64_t)ptr; i < (uint64_t)ptr + PAGE_SIZE * 16;
       i += kmem_size_classes[class]) {
    INIT_LIST_HEAD((list_head_t *)i);
    list_add((list_head_t *)i, &slab->free_list);
    slab->total++;
  }

  for (int i = 0; i < 16; i++) {
    page_t *page = addr_to_page((uint64_t)ptr + i * PAGE_SIZE);
    page->slab = slab;
  }
  return slab;
}

kmem_slab_t *kmem_alloc_slab_small(uint32_t class) {
  // Alloc memory from buddy allocator
  void *ptr = mm_alloc(PAGE_SIZE);

  // setup slab
  kmem_slab_t *slab = (kmem_slab_t *)(ptr + PAGE_SIZE - sizeof(kmem_slab_t));
  INIT_LIST_HEAD(&slab->free_list);
  INIT_LIST_HEAD(&slab->list);
  slab->slab_base = ptr;
  slab->inuse = 0;
  slab->total = 0;
  slab->class = class;

  // init free list
  for (uint64_t i = (uint64_t)ptr;
       i < (uint64_t)(ptr + PAGE_SIZE - sizeof(kmem_slab_t) -
                      kmem_size_classes[class]);
       i += kmem_size_classes[class]) {
    INIT_LIST_HEAD((list_head_t *)i);
    list_add((list_head_t *)i, &slab->free_list);
    slab->total++;
  }

  // steup page
  page_t *page = addr_to_page(ptr);
  page->slab = slab;
  return slab;
}

void kmem_cache_init() {
  for (int i = 0; i < KMEM_SIZE_CLASS; i++) {
    kmem_caches[i].slab_order = 0;
    kmem_caches[i].free_list_size = 0;
    INIT_LIST_HEAD(&kmem_caches[i].free_list);
    INIT_LIST_HEAD(&kmem_caches[i].partial_list);
    INIT_LIST_HEAD(&kmem_caches[i].full_list);
  }
}

void *kmem_cache_alloc(uint32_t class) {
  if (class >= KMEM_SIZE_CLASS) {
    return NULL;
  }

  // allocate a new slab
  if (list_empty(&kmem_caches[class].partial_list)) {
    if (!list_empty(&kmem_caches[class].free_list)) {
      kmem_slab_t *slab =
          list_entry(kmem_caches[class].free_list.next, kmem_slab_t, list);
      kmem_caches[class].free_list_size--;

      list_del(&slab->list);
      list_add(&slab->list, &kmem_caches[class].partial_list);
    } else {
      kmem_slab_t *new_slab = class < 5 ? kmem_alloc_slab_small(class)
                                        : kmem_alloc_slab_large(class);
      list_add(&new_slab->list, &kmem_caches[class].partial_list);
    }
  }
  kmem_slab_t *slab =
      list_entry(kmem_caches[class].partial_list.next, kmem_slab_t, list);
  if (slab->inuse == slab->total - 1) {
    // printf("[kmem] slab %p is full\r\n", slab);
    list_del(&slab->list);
    list_add(&slab->list, &kmem_caches[class].full_list);
  }
  slab->inuse++;

  void *ptr = slab->free_list.next;
  list_del(slab->free_list.next);

  return ptr;
}

void kmem_cache_free(void *ptr) {
  // find the page
  page_t *page = addr_to_page(ptr);

  // find the slab
  kmem_slab_t *slab = (kmem_slab_t *)page->slab;
  INIT_LIST_HEAD((list_head_t *)ptr);
  list_add((list_head_t *)ptr, &slab->free_list);

  // check if slab is full
  if (slab->inuse == slab->total) {
    list_del(&slab->list);
    list_add(&slab->list, &kmem_caches[slab->class].partial_list);
  }
  slab->inuse--;
  if (slab->inuse == 0) {
    list_del(&slab->list);
    if (kmem_caches[slab->class].free_list_size < 2) {
      list_add(&slab->list, &kmem_caches[slab->class].free_list);
      kmem_caches[slab->class].free_list_size++;
    } else {
      mm_free(slab->slab_base);
      // because slab struct is allocated from slab allocator
      if (slab->class >= 5) {
        kfree(slab);
      }
    }
  }
}

void *kmalloc(uint32_t size) {
  // slab allocation
  for (uint32_t i = 0; i < KMEM_SIZE_CLASS; i++) {
    if (size <= kmem_size_classes[i]) {
      return kmem_cache_alloc(i);
    }
  }
  // handle for bigger memory allocation
  void *ptr = mm_alloc(size);
  page_t *page = addr_to_page(ptr);
  page->slab = NULL;
  return ptr;
}

void kfree(void *ptr) {
  page_t *page = addr_to_page(ptr);
  if (page->slab == NULL) {
    // handle for bigger memory free
    mm_free(ptr);
  } else {
    // slab free
    kmem_cache_free(ptr);
  }
}
