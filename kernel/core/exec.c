#include "core/exec.h"
#include "common/printf.h"
#include "common/types.h"
#include "common/utils.h"
#include "core/sched.h"
#include "core/task.h"
#include "fs/file.h"
#include "mm/slab.h"

void user_exec(void *prog, uint32_t prog_len) {
  uint64_t *prog_ptr = (uint64_t *)prog;
  uint64_t program_addr = USER_SPACE_BEGIN;
  for (uint32_t i = 0; i < prog_len / 8; i++) {
    ((uint64_t *)program_addr)[i] = prog_ptr[i];
  }
  program_addr = USER_SPACE_BEGIN;

  uint64_t stack_begin = USER_STACK_END;
  asm volatile("mov x0, %0\n\t"
               "msr sp_el0, x0\n\t"
               :
               : "r"(stack_begin));

  asm volatile("mov x0, %0\n\t"
               "msr elr_el1, x0\n\t"
               :
               : "r"(program_addr));

  asm volatile("mov x0, #0x340\n\t"
               "msr spsr_el1, x0\n\t");

  asm volatile("eret");
}

int32_t do_exec(const char *filename, char *const argv[]) {
  void *user_prog;
  int32_t prog_len = file_find(filename, (char **)&user_prog);
  if (prog_len == -1) {
    printf("File not found: %s\r\n", filename);
    return -1;
  }

  // 1. Get the current task
  task_struct_t *task = get_current();

  // 2. Free old program and user stack
  if (task->prog) {
    kfree(task->prog);
  }
  if (task->user_stack) {
    kfree(task->user_stack);
  }

  // 3. Allocate new program and user stack
  task->prog = kmalloc(prog_len);
  if (!task->prog) {
    printf("Failed to allocate program memory\n");
    return -1;
  }
  task->prog_size = prog_len;
  memcpy(task->prog, user_prog, prog_len);

  task->user_stack = kmalloc(TASK_STACK_SIZE);
  if (!task->user_stack) {
    printf("Failed to allocate user stack\n");
    kfree(task->prog);
    return -1;
  }

  // 4. Reset the task type
  task->type = TASK_USER;

  // 5. Set up the trapframe
  trapframe_t *trapframe =
      (trapframe_t *)((uint8_t *)task->stack + TASK_STACK_SIZE -
                      sizeof(trapframe_t));
  task->trapframe = trapframe;
  task->context.sp = (uint64_t)task->trapframe;
  task->context.lr = (uint64_t)task_return_el0;
  task->irq_priority = 0x3f3f3f3f;

  memset(trapframe, 0, sizeof(trapframe_t));
  trapframe->elr_el1 = (uint64_t)task->prog;
  trapframe->spsr_el1 = 0x340; // Set the SPSR to EL1h
  trapframe->sp_el0 = (uint64_t)task->user_stack + TASK_STACK_SIZE;

  // 6. Handle arguments (for now, we don't do anything with them)
  // if (argv != NULL) {
  //   int argc = 0;
  //   while (argv[argc] != NULL) {
  //     trapframe->gpr[argc] = (uint64_t)argv[argc];
  //     argc++;
  //   }
  // }

  // 7. reset signal
  memset(task->signal.handler, 0, sizeof(task->signal.handler));

  // 8. return to user space
  static task_struct_t dummy;
  switch_to(&dummy, task);

  while (1) // should never reach here
    ;
  return 0;
}
