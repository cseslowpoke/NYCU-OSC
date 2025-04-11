#include "core/slab.h"
#include "common/list.h"
#include "common/printf.h"
#include "common/types.h"
#include "core/mm.h"

static const uint32_t kmem_size_classes[] = {16,  32,   64,   128,  256,
                                             512, 1024, 2048, 4096, 8192};

static kmem_cache_t kmem_caches[KMEM_SIZE_CLASS];

kmem_slab_t *kmem_alloc_slab(uint32_t class) {
  void *ptr = mm_alloc(PAGE_SIZE);
  kmem_slab_t *slab = (kmem_slab_t *)(ptr + PAGE_SIZE - sizeof(kmem_slab_t));
  INIT_LIST_HEAD(&slab->free_list);
  INIT_LIST_HEAD(&slab->next);
  slab->inuse = 0;
  slab->class = class;
  for (uint64_t i = (uint64_t)ptr;
       i < (uint64_t)(ptr + PAGE_SIZE - sizeof(kmem_slab_t) -
                      kmem_size_classes[class]);
       i += kmem_size_classes[class]) {
    INIT_LIST_HEAD((list_head_t *)i);
    list_add((list_head_t *)i, &slab->free_list);
    slab->total++;
  }
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
    kmem_slab_t *new_slab = kmem_alloc_slab(class);
    list_add(&new_slab->next, &kmem_caches[class].partial_list);
  }
  kmem_slab_t *slab =
      list_entry(kmem_caches[class].partial_list.next, kmem_slab_t, next);
  if (slab->inuse == slab->total - 1) {
    list_del(&slab->next);
    list_add(&slab->next, &kmem_caches[class].full_list);
  }
  slab->inuse++;

  void *ptr = slab->free_list.next;
  list_del(slab->free_list.next);

  return ptr;
}

void kmem_cache_free(void *ptr) {
  // find the slab
  kmem_slab_t *slab = (kmem_slab_t *)(((uint64_t)ptr & ~(PAGE_SIZE - 1)) +
                                      PAGE_SIZE - sizeof(kmem_slab_t));
  INIT_LIST_HEAD((list_head_t *)ptr);
  list_add((list_head_t *)ptr, &slab->free_list);

  if (slab->inuse == slab->total) {
    list_del(&slab->next);
    list_add(&slab->next, &kmem_caches[slab->class].partial_list);
  }
  slab->inuse--;
  if (slab->inuse == 0) {
    list_del(&slab->next);
    if (kmem_caches[slab->class].free_list_size < 2) {
      list_add(&slab->next, &kmem_caches[slab->class].free_list);
      kmem_caches[slab->class].free_list_size++;
    } else {
      mm_free(slab);
    }
  }
}

void *kmalloc(uint32_t size) {
  uint32_t class = 0;
  for (uint32_t i = 0; i < KMEM_SIZE_CLASS; i++) {
    if (size <= kmem_size_classes[i]) {
      class = i;
      break;
    }
  }
  return kmem_cache_alloc(class);
}

void kfree(void *ptr) { kmem_cache_free(ptr); }
