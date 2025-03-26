#include "core/user_exec.h"
#include "common/types.h"

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
