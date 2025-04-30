#include "core/task.h"
#include "common/list.h"
#include "common/types.h"
#include "common/utils.h"
#include "mm/slab.h"

static uint64_t next_pid = 1;

task_struct_t *task_create_kernel(void (*fn)(void)) {
  task_struct_t *task = kmalloc(sizeof(task_struct_t));
  task->pid = next_pid++;
  task->state = TASK_SLEEPING;
  task->type = TASK_KERNEL;
  task->stack = kmalloc(TASK_STACK_SIZE); // Allocate a stack of 4KB
  task->context.sp =
      (uint64_t)(task->stack + TASK_STACK_SIZE); // Set stack pointer
  task->context.fp = (uint64_t)(task->stack + TASK_STACK_SIZE);
  task->context.lr = (uint64_t)task_entry_wrapper; // Set link register
  task->fn = fn;                    // Set the function to be executed
  task->irq_priority = 0x3f3f3f3f;  // Set the default IRQ priority
  INIT_LIST_HEAD(&task->task_list); // Initialize the task list
  return task;
}

task_struct_t *task_create_user() {
  task_struct_t *task = kmalloc(sizeof(task_struct_t));
  task->pid = next_pid++;
  task->state = TASK_SLEEPING;
  task->type = TASK_USER;
  task->stack = kmalloc(TASK_STACK_SIZE); // Allocate a stack of 4KB
  task->context.sp =
      (uint64_t)(task->stack + TASK_STACK_SIZE); // Set stack pointer
  task->context.fp = (uint64_t)(task->stack + TASK_STACK_SIZE);
  task->context.lr = (uint64_t)task_return_el0;
  task->user_stack = kmalloc(TASK_STACK_SIZE); // Allocate a user stack
  task->irq_priority = 0x3f3f3f3f;             // Set the default IRQ priority
  INIT_LIST_HEAD(&task->task_list);            // Initialize the task list
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
    // DELAY_CYCLES(1000000);
    sched(); // Yield to the scheduler
  }
}

#include "core/shell.h"

void task_test() {
  for (int i = 0; i < 5; i++) {
    task_struct_t *task = task_create_kernel(foo);
    sched_add(task);
  }
  // task_struct_t *task = task_create_kernel(shell_start);
  // sched_add(task);

  sched_idle();
}
