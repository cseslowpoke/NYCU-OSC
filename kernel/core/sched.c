#include "core/sched.h"
#include "common/list.h"
#include "common/printf.h"
#include "common/utils.h"
#include "core/task.h"
#include "drivers/irq.h"
#include "drivers/timer.h"
#include "mm/mmu.h"
#include "mm/slab.h"

static list_head_t task_list;
static list_head_t zombie_list;

__attribute__((naked)) void switch_to(task_struct_t *prev,
                                      task_struct_t *next) {
  __asm__ volatile("stp x19, x20, [x0, #16 * 0] \n"
                   "stp x21, x22, [x0, #16 * 1] \n"
                   "stp x23, x24, [x0, #16 * 2] \n"
                   "stp x25, x26, [x0, #16 * 3] \n"
                   "stp x27, x28, [x0, #16 * 4] \n"
                   "stp x29, x30, [x0, #16 * 5] \n"
                   "mov x9, sp \n"
                   "str x9, [x0, #16 * 6] \n"

                   "ldp x19, x20, [x1, #16 * 0] \n"
                   "ldp x21, x22, [x1, #16 * 1] \n"
                   "ldp x23, x24, [x1, #16 * 2] \n"
                   "ldp x25, x26, [x1, #16 * 3] \n"
                   "ldp x27, x28, [x1, #16 * 4] \n"
                   "ldp x29, x30, [x1, #16 * 5] \n"
                   "ldr x9, [x1, #16 * 6] \n"
                   "mov sp, x9 \n"
                   "msr tpidr_el1, x1\n"

                   "ret \n");
}

extern char _stack_top[];

void sched_time_slice() {
  // printf("sched_time_slice\r\n");
  timer_add_task((timer_handler_t)sched_time_slice, NULL,
                 READ_SYSREG(CNTFRQ_EL0) >> 5);
  // timer_add_task((timer_handler_t)sched_time_slice, NULL,
  //                READ_SYSREG(CNTFRQ_EL0));
  sched();
}

void sched_init() {
  INIT_LIST_HEAD(&task_list);
  INIT_LIST_HEAD(&zombie_list);

  // create the main kernel task
  task_struct_t *current = kmalloc(sizeof(task_struct_t));
  current->pid = 0;
  current->state = TASK_RUNNING;
  current->type = TASK_KERNEL;
  current->stack = _stack_top;
  INIT_LIST_HEAD(&current->task_list);
  WRITE_SYSREG(TPIDR_EL1, current);
  timer_add_task((timer_handler_t)sched_time_slice, NULL,
                 READ_SYSREG(CNTFRQ_EL0) >> 5);
}

void sched() {
  DISABLE_IRQ();
  if (list_empty(&task_list)) {
    return; // No tasks to schedule
  }

  task_struct_t *current = get_current();
  task_struct_t *next;
  do {
    if (list_empty(&task_list)) {
      ENABLE_IRQ();
      return; // No tasks to schedule
    }
    next = list_entry(task_list.next, task_struct_t, task_list);
    list_del(&next->task_list);
    if (next->state != TASK_ZOMBIE) {
      break;
    }
    list_add_tail(&next->task_list, &zombie_list);
  } while (1);

  next->state = TASK_RUNNING;

  if (current->state != TASK_ZOMBIE) {
    list_add_tail(&current->task_list, &task_list);
    current->state = TASK_SLEEPING;
  } else {
    list_add_tail(&current->task_list, &zombie_list);
  }

  ENABLE_IRQ();

  if (next->type == TASK_USER) {
    mmu_switch_to(next->pgd);
  }
  switch_to(current, next);

  // here is the next task
  current = get_current();

  if (current->type == TASK_USER) {
    signal_handler();
  }
}

void sched_add(task_struct_t *task) {
  DISABLE_IRQ();
  list_add(&task->task_list, &task_list);
  ENABLE_IRQ();
  task->state = TASK_SLEEPING;
}

void sched_kill_zombie() {
  while (!list_empty(&zombie_list)) {
    task_struct_t *task =
        list_entry(zombie_list.next, task_struct_t, task_list);
    list_del(&task->task_list);
    kfree(task->stack);
    kfree(task);
  }
}

void sched_idle() {
  while (1) {
    DISABLE_IRQ();
    sched();
    sched_kill_zombie();
  }
}

void sched_kill_task(uint32_t pid) {
  list_head_t *pos;
  list_for_each(pos, &task_list) {
    task_struct_t *task = list_entry(pos, task_struct_t, task_list);
    if (task->pid == pid) {
      if (task->type == TASK_KERNEL) {
        printf("Cannot kill kernel task\r\n");
        return; // Cannot kill kernel task
      }
      task->state = TASK_ZOMBIE;
      return;
    }
  }
}

task_struct_t *sched_get_task_by_pid(uint32_t pid) {
  task_struct_t *current = get_current();
  if (current->pid == pid) {
    return current;
  }
  list_head_t *pos;
  list_for_each(pos, &task_list) {
    task_struct_t *task = list_entry(pos, task_struct_t, task_list);
    if (task->pid == pid) {
      return task;
    }
  }
  return NULL;
}
