#include "mm/mmu.h"
#include "common/printf.h"
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
      page_t *page = addr_to_page(new_pt);
      page->ref_count = 1;
      pg_t[idx] = virt_to_phy(new_pt) | PD_TABLE;
    }
    pg_t = (uint64_t *)phy_to_virt(pg_t[idx] & ~0xfff);
  }
  int idx = (va >> level_shift[LEVEL - 1]) & 0x1ff;
  return &pg_t[idx];
}

uint64_t *mmu_walk_cow(uint64_t *pg_t, uint64_t va) {
  for (int level = 0; level < LEVEL - 1; level++) {
    int idx = (va >> level_shift[level]) & 0x1ff;
    uint64_t entry = pg_t[idx];
    page_t *page = addr_to_page(phy_to_virt(entry & ~0xfff));
    if (page->ref_count > 1) {
      page->ref_count--;
      uint64_t *new_pt = kmalloc(PAGE_SIZE);
      memcpy(new_pt, (void *)phy_to_virt(entry & ~0xfff), PAGE_SIZE);
      page_t *new_page = addr_to_page(new_pt);
      new_page->ref_count = 1;
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
  page_t *page = addr_to_page(pg_t);
  page->ref_count = 1;
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

void vma_insert(task_struct_t *task, vm_area_t *vma) {
  if (list_empty(&task->vm_area_list)) {
    list_add(&task->vm_area_list, &vma->list);
  } else {
    list_head_t *pos;
    list_for_each(pos, &task->vm_area_list) {
      vm_area_t *cur = list_entry(pos, vm_area_t, list);
      if (vma->end < cur->start) {
        list_add_tail(&vma->list, pos);
        break;
      }
    }
    if (list_empty(&vma->list)) {
      list_add_tail(&vma->list, &task->vm_area_list);
    }
  }
}

vm_area_t *vma_find(task_struct_t *task, uint64_t va) {
  vm_area_t *ret = NULL;
  list_head_t *pos;
  list_for_each(pos, &task->vm_area_list) {
    vm_area_t *cur = list_entry(pos, vm_area_t, list);
    if (va >= cur->start && va < cur->end) {
      ret = cur;
      break;
    }
  }
  return ret;
}

int64_t vma_find_unmapped_area_near(task_struct_t *task, uint64_t addr,
                                    uint64_t size) {
  addr = round_down(addr, PAGE_SIZE);
  size = round_up(size, PAGE_SIZE);

  int64_t ret = -1;

  // try to find not overlap area
  list_head_t *pos, *prev = NULL;
  list_for_each(pos, &task->vm_area_list) {
    if (prev != NULL) {
      vm_area_t *cur = list_entry(pos, vm_area_t, list);
      vm_area_t *prev_vma = list_entry(prev, vm_area_t, list);
      if (addr + size <= cur->start && addr >= prev_vma->end) {
        ret = addr;
        goto exit;
      }
    }
    prev = pos;
  }

  // try to find area
  prev = NULL;
  list_for_each(pos, &task->vm_area_list) {
    if (prev != NULL) {
      vm_area_t *cur = list_entry(pos, vm_area_t, list);
      vm_area_t *prev_vma = list_entry(prev, vm_area_t, list);
      if (prev_vma->end + size <= cur->start) {
        ret = prev_vma->end;
        goto exit;
      }
    }
    prev = pos;
  }

exit:
  return ret;
}

void do_page_fault(uint64_t fault_addr, task_struct_t *task) {
  DISABLE_IRQ();
  vm_area_t *vma = vma_find(task, fault_addr);
  if (vma == NULL) {
    debug_printf("[Segmentation fault]: Kill Process, FAR: 0x%x\r\n",
                 fault_addr);
    while (1) {
      __asm__ volatile("wfi");
    }
  } else {
    fault_addr = round_down(fault_addr, PAGE_SIZE);
    uint64_t offset = fault_addr - vma->start;
    printf("[Translation fault]: 0x%x\r\n", fault_addr);
    uint64_t *pa = kmalloc(0x1000);
    memset(pa, 0, PAGE_SIZE);

    if (vma->file != NULL) {
      memcpy(pa, vma->file + vma->offset + offset, PAGE_SIZE);
    }
    uint64_t *entry = mmu_walk(task->pgd, fault_addr, 1);
    *entry = virt_to_phy(pa) | vma->prot | PD_PAGE_ENTRY;
  }
}

void do_permission_fault(uint64_t fault_addr, task_struct_t *task) {
  DISABLE_IRQ();

  vm_area_t *vma = vma_find(task, fault_addr);
  if (vma == NULL) {
    printf("wtf");
    while (1) {
      __asm__ volatile("wfi");
    }
  } else {
    if ((vma->prot & (0b11 << 6)) == AP_RW_EL0) {
      debug_printf("[Copy on write]: %x\r\n", fault_addr);
      uint64_t *old_entry = mmu_walk(task->pgd, fault_addr, 0);
      uint64_t *new_entry = mmu_walk_cow(task->pgd, fault_addr);
      void *old_pa = (void *)phy_to_virt(*old_entry & ~0xfff & ~PD_UXN);
      void *new_pa = kmalloc(PAGE_SIZE);
      memcpy(new_pa, old_pa, PAGE_SIZE);
      new_entry[0] = virt_to_phy(new_pa) | vma->prot | PD_PAGE_ENTRY;
      old_entry[0] = virt_to_phy(old_pa) | vma->prot | PD_PAGE_ENTRY;
      __asm__ volatile("dsb ish\n\t"
                       // "tlbi vae1, %0\n\t"
                       "tlbi vmalle1is\n\t"
                       "dsb ish\n\t"
                       "isb\n\t"
                       :
                       : "r"(fault_addr)
                       : "memory");
    } else {
      debug_printf("[Permission fault]: %x\r\n", fault_addr);
      while (1) {
        __asm__ volatile("wfi");
      }
    }
  }
}

void increase_ref_count(uint64_t *pgd, int level) {
  for (int i = 0; i < 512; i++) {
    if (pgd[i] & PD_TABLE) {
      uint64_t *pt = (uint64_t *)(pgd[i] & ~0xfff);
      if (0x3c000000 <= (uint64_t)pt && (uint64_t)pt < 0x3f000000) {
        continue;
      }

      pt = (uint64_t *)phy_to_virt(pt);
      page_t *page = addr_to_page(pt);
      page->ref_count++;
      if (level < 3) {
        increase_ref_count(pt, level + 1);
      }
    }
  }
}

void vma_mark_readonly(uint64_t *pgd, vm_area_t *area) {
  for (uint64_t i = area->start; i < area->end; i += PAGE_SIZE) {
    uint64_t *entry = mmu_walk(pgd, i, 0);
    if (entry) {
      *entry = (*entry) | AP_RO_EL1;
    }
  }
}
