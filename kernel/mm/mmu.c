#include "mm/mmu.h"
#include "common/types.h"
#include "common/utils.h"
#include "drivers/irq.h"
#include "mm/mm.h"

void mmu_init_kernel() {
  DISABLE_IRQ();
  uint64_t *page_table_l2_0 = kmalloc(PAGE_SIZE);

  // first entry of pmd
  // map normal memory [0x0, 0x3f000000)
  for (int i = 0; i < 0x3f000000 / 0x200000; i++) {
    uint64_t pa = i * 0x200000;
    page_table_l2_0[i] = pa | PD_ACCESS | (MAIR_NORMAL_NOCACHE << 2) | PD_BLOCK;
  }
  // map device memory [0x3f000000, 0x40000000)
  for (int i = 0x3f000000 / 0x200000; i < 0x40000000 / 0x200000; i++) {
    uint64_t pa = i * 0x200000;
    page_table_l2_0[i] =
        pa | PD_ACCESS | (MAIR_DEVICE_nGnRnE << 2) | PD_BLOCK | PD_PXN | PD_UXN;
  }

  uint64_t *page_table_l2_1 = kmalloc(PAGE_SIZE);
  // second entry of pmd
  // map device memory [0x40000000, 0x80000000)
  for (int i = 0; i < 0x40000000 / 0x200000; i++) {
    uint64_t pa = i * 0x200000 + 0x40000000;
    page_table_l2_1[i] =
        pa | PD_ACCESS | (MAIR_DEVICE_nGnRnE << 2) | PD_BLOCK | PD_PXN | PD_UXN;
  }
  KERNEL_IDENTITY_L1[0] = virt_to_phy(page_table_l2_0) | PD_TABLE;
  KERNEL_IDENTITY_L1[1] = virt_to_phy(page_table_l2_1) | PD_TABLE;
  ENABLE_IRQ();
}

static int level_shift[4] = {39, 30, 21, 12};

uint64_t *mmu_walk(uint64_t *pg_t, uint64_t va, uint64_t alloc) {
  for (int level = 0; level < LEVEL - 1; level++) {
    int idx = (va >> level_shift[level]) & 0x1ff;
    uint64_t entry = pg_t[idx];
    if ((entry & 0b11) != PD_TABLE) {
      if (!alloc) {
        return NULL;
      }
      uint64_t *new_pt = kmalloc(PAGE_SIZE);
      memset(new_pt, 0, PAGE_SIZE);
      pg_t[idx] = virt_to_phy(new_pt) | PD_TABLE;
    }
    pg_t = (uint64_t *)phy_to_virt(pg_t[idx] & ~0xfff);
  }
  int idx = (va >> level_shift[LEVEL - 1]) & 0x1ff;
  return &pg_t[idx];
}

int mmu_map(uint64_t *pg_t, uint64_t va, uint64_t pa, uint64_t size,
            uint64_t attr) {
  for (uint64_t i = 0; i < size; i += PAGE_SIZE) {
    uint64_t *entry = mmu_walk(pg_t, va + i, 1);
    if (!entry) {
      return -1;
    }
    *entry = (pa + i) | attr | PD_PAGE_ENTRY;
  }
  return 0;
}

uint64_t *mmu_create_pg() {
  uint64_t *pg_t = kmalloc(PAGE_SIZE);
  memset(pg_t, 0, PAGE_SIZE);
  return pg_t;
}

void mmu_switch_to(uint64_t *pgd) {
  __asm__ volatile(
      "mov x0, %0\n\t" // Load the pgd
      "dsb ish\n\t"
      "msr ttbr0_el1, x0\n\t" // Set the translation table base register
      "tlbi vmalle1is\n\t"
      "dsb ish\n\t"
      "isb\n\t"
      :
      : "r"(virt_to_phy(pgd))
      : "x0", "memory");
}
