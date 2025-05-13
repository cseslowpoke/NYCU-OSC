#ifndef __MMU_H
#define __MMU_H

#include "common/types.h"

#define LEVEL 4

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

#endif
