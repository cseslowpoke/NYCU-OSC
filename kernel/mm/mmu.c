#include "mm/mmu.h"
#include "common/types.h"
#include "common/utils.h"
#include "mm/mm.h"

void mmu_init_kernel() {
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
}
