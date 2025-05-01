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
  memset(task, 0, sizeof(task_struct_t)); // Clear the task structure
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
  memset(task, 0, sizeof(task_struct_t)); // Clear the task structure
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

void task_destory(task_struct_t *task) {
  if (task->type == TASK_USER) {
    kfree(task->user_stack); // Free the user stack
    kfree(task->prog);       // Free the program memory
  }
  kfree(task->stack); // Free the kernel stack
  kfree(task);        // Free the task structure
}

void task_entry_wrapper() {
  task_struct_t *task = get_current();
  task->fn();      // Call the task function
  task_exit(task); // Destroy the task
}

__attribute__((naked)) void task_return_el0() {
  __asm__ volatile("ldr x0, [sp, #0]\n\t"
                   "msr elr_el1, x0\n\t" // set elr_el1 to return value
                   "ldr x0, [sp, #8]\n\t"
                   "msr spsr_el1, x0\n\t" // set spsr_el1 to return value
                   "ldr x0, [sp, #16]\n\t"
                   "msr sp_el0, x0\n\t" // set sp_el0 to return value
                   "ldp x0, x1, [sp, #24]\n\t"
                   "ldp x2, x3, [sp, #(3 * 8) + 16 * 1]\n\t"
                   "ldp x4, x5, [sp, #(3 * 8) + 16 * 2]\n\t"
                   "ldp x6, x7, [sp, #(3 * 8) + 16 * 3]\n\t"
                   "ldp x8, x9, [sp, #(3 * 8) + 16 * 4]\n\t"
                   "ldp x10, x11, [sp, #(3 * 8) + 16 * 5]\n\t"
                   "ldp x12, x13, [sp, #(3 * 8) + 16 * 6]\n\t"
                   "ldp x14, x15, [sp, #(3 * 8) + 16 * 7]\n\t"
                   "ldp x16, x17, [sp, #(3 * 8) + 16 * 8]\n\t"
                   "ldp x18, x19, [sp, #(3 * 8) + 16 * 9]\n\t"
                   "ldp x20, x21, [sp, #(3 * 8) + 16 * 10]\n\t"
                   "ldp x22, x23, [sp, #(3 * 8) + 16 * 11]\n\t"
                   "ldp x24, x25, [sp, #(3 * 8) + 16 * 12]\n\t"
                   "ldp x26, x27, [sp, #(3 * 8) + 16 * 13]\n\t"
                   "ldp x28, x29, [sp, #(3 * 8) + 16 * 14]\n\t"
                   "ldr x30, [sp, #(3 * 8) + 16 * 15]\n\t"

                   "add sp, sp, 34 * 8\n\t" // pop 16 * 8 = 128 bytes
                   "eret\n\t");
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
