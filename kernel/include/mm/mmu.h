#ifndef __MMU_H
#define __MMU_H

#include "common/list.h"
#include "common/types.h"
#include "core/task.h"

#define LEVEL 4
#define PAGE_SIZE 0x1000

// Entry type
#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_PAGE_ENTRY 0b11

// MAIR idx
#define MAIR_NORMAL (0b00 << 2)
#define MAIR_DEVICE (0b01 << 2)

// Access permission
#define AP_RW_EL1 (0b00 << 6)
#define AP_RW_EL0 (0b01 << 6)
#define AP_RO_EL1 (0b10 << 6)
#define AP_RO_EL0 (0b11 << 6)

// Access Flag
#define PD_ACCESS (1 << 10)

#define PD_PXN (1ull << 53)
#define PD_UXN (1ull << 54)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100

#define KERNEL_IDENTITY_L0 ((uint64_t *)0xffff000000001000)
#define KERNEL_IDENTITY_L1 ((uint64_t *)0xffff000000002000)

#define virt_to_phy(x) (((uint64_t)x) & 0xffffffffffff)
#define phy_to_virt(x) (((uint64_t)x) | 0xffff000000000000)

// Setup more granularity mapping
// Normal Memory [0x0, 0x3f000000)
// Device Memory [0x3f000000, 0x80000000)
void mmu_init_kernel();

uint64_t *mmu_walk(uint64_t *pg_t, uint64_t va, uint64_t alloc);

int mmu_map(uint64_t *pg_t, uint64_t va, uint64_t pa, uint64_t size,
            uint64_t attr);

uint64_t *mmu_create_pg();

void mmu_switch_to(uint64_t *pgd);

typedef struct vm_area {
  // Basic VMA
  uint64_t start;
  uint64_t end;
  uint64_t prot;
  uint64_t flags;

  // File Extension
  void *file;
  uint64_t offset;

  // List
  list_head_t list;
} vm_area_t;

void vma_insert(task_struct_t *task, vm_area_t *vma);

vm_area_t *vma_find(task_struct_t *task, uint64_t addr);

int64_t vma_find_unmapped_area_near(task_struct_t *task, uint64_t addr,
                                    uint64_t size);

void vma_mark_readonly(uint64_t *pgd, vm_area_t *area);

void do_page_fault(uint64_t fault_addr, task_struct_t *task);

void do_permission_fault(uint64_t fault_addr, task_struct_t *task);

void increase_ref_count(uint64_t *pg_t, int level);

#endif
