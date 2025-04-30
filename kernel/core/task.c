#include "core/task.h"
#include "common/list.h"
#include "common/printf.h"
#include "common/types.h"
#include "common/utils.h"
#include "core/sched.h"
#include "core/shell.h"
#include "mm/slab.h"

static uint32_t next_pid = 1;

static task_struct_t *task_init_common(task_struct_t *task,
                                       enum task_type type) {
  task->pid = next_pid++;
  task->state = TASK_SLEEPING;
  task->type = type;
  task->stack = kmalloc(TASK_STACK_SIZE); // Allocate a stack of 4KB
  if (!task->stack) {
    printf("Failed to allocate stack for task %d\n", task->pid);
    return NULL;
  }
  task->context.sp = (uint64_t)(task->stack + TASK_STACK_SIZE);
  task->context.fp = (uint64_t)(task->stack + TASK_STACK_SIZE);
  task->irq_priority = 0x3f3f3f3f;  // Set the default IRQ priority
  INIT_LIST_HEAD(&task->task_list); // Initialize the task list
  return task;
}

task_struct_t *task_create_kernel(void (*fn)(void)) {
  task_struct_t *task = kmalloc(sizeof(task_struct_t));
  if (!task) {
    printf("Failed to allocate task_struct\n");
    return NULL;
  }
  if (!task_init_common(task, TASK_KERNEL)) {
    kfree(task);
    return NULL;
  }
  task->context.lr = (uint64_t)task_entry_wrapper; // Set link register
  task->fn = fn; // Set the function to be executed
  return task;
}

task_struct_t *task_create_user() {
  task_struct_t *task = kmalloc(sizeof(task_struct_t));
  if (!task) {
    printf("Failed to allocate task_struct\n");
    return NULL;
  }
  if (!task_init_common(task, TASK_USER)) {
    kfree(task);
    return NULL;
  }
  task->context.lr = (uint64_t)task_return_el0;
  task->user_stack = kmalloc(TASK_STACK_SIZE); // Allocate a user stack
  return task;
}

void task_exit(task_struct_t *task) {
  task->state = TASK_ZOMBIE; // Mark the task as a zombie
  sched();
}

void task_entry_wrapper() {
  task_struct_t *task = get_current();
  task->fn();      // Call the task function
  task_exit(task); // Destroy the task
}

void task_return_el0() {
  // TODO: implement this
  task_struct_t *task = get_current();
  asm volatile("mov x0, %0\n\t"
               "msr sp_el0, x0\n\t"
               :
               : "r"(task->trapframe->sp_el0));
  asm volatile("mov x0, %0\n\t"
               "msr elr_el1, x0\n\t"
               :
               : "r"(task->trapframe->elr_el1));
  // asm volatile("mov x0, #0x340\n\t"
  //              "msr spsr_el1, x0\n\t");
  asm volatile("mov x0, %0\n\t"
               "msr spsr_el1, x0\n\t"
               :
               : "r"(task->trapframe->spsr_el1));
  asm volatile("eret");
  while (1)
    ; // Should never reach here
}

void foo() {
  for (int i = 0; i < 10; i++) {
    task_struct_t *task = get_current();
    printf("Task %d: %d\r\n", task->pid, i);
    DELAY_CYCLES(1000000);
    sched(); // Yield to the scheduler
  }
}

void task_test() {
  // for (int i = 0; i < 5; i++) {
  //   task_struct_t *task = task_create_kernel(foo);
  //   sched_add(task);
  // }
  task_struct_t *task = task_create_kernel(shell_start);
  sched_add(task);
  // do_exec("fork.img", NULL);

  sched_idle();
}
