#include "core/fork.h"
#include "common/types.h"
#include "core/exec.h"
#include "core/sched.h"
#include "core/syscall.h"
#include "core/task.h"
#include "mm/mm.h"
#include "mm/mmu.h"
#include "mm/slab.h"

struct signal_info_t;

int32_t do_fork() {
  task_struct_t *current = get_current();
  if (!current) {
    return -1;
  }

  task_struct_t *child = task_create_user();
  if (!child) {
    return -1;
  }

  child->prog_size = current->prog_size;
  child->prog = current->prog;

  // copy current stack to child
  // memcpy(child->user_stack, current->user_stack, USER_STACK_SIZE);

  child->state = TASK_SLEEPING;
  child->trapframe = child->stack + KERNEL_STACK_SIZE -
                     sizeof(trapframe_t); // set trapframe to child stack
  memcpy(child->trapframe, current->trapframe, sizeof(trapframe_t));

  child->trapframe->gpr[0] = 0;

  child->pgd = mmu_create_pg();
  increase_ref_count(current->pgd, 0);
  memcpy(child->pgd, current->pgd, PAGE_SIZE);

  INIT_LIST_HEAD(&child->vm_area_list);
  list_head_t *vma_pos;
  list_for_each(vma_pos, &current->vm_area_list) {
    vm_area_t *cur_vma = list_entry(vma_pos, vm_area_t, list);
    vm_area_t *new_vma = kmalloc(sizeof(vm_area_t));
    memcpy(new_vma, cur_vma, sizeof(vm_area_t));
    INIT_LIST_HEAD(&new_vma->list);
    vma_insert(child, new_vma);
    if (new_vma->start != 0x3c000000 && new_vma->end != 0x3f000000 &&
        ((new_vma->prot & (0b11 << 6)) == AP_RW_EL0)) {
      vma_mark_readonly(child->pgd, new_vma);
    }
  }

  child->context.sp = (uint64_t)child->trapframe;
  child->context.fp = (uint64_t)child->trapframe;
  child->context.lr = (uint64_t)task_return_el0;

  // copy parent signal to child
  memcpy(&child->signal, &current->signal,
         sizeof(signal_info_t)); // copy signal struct

  // Add child to the scheduler
  sched_add(child);

  return child->pid;
}
