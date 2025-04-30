#include "core/exception.h"
#include "common/printf.h"
#include "common/types.h"
#include "common/utils.h"
#include "core/syscall.h"
#include "core/task.h"
#include "drivers/irq.h"

void print_exception_imformation(trapframe_t *tf) {
  // task_struct_t *current = get_current();
  // printf("Exception in task pid %d\r\n", current->pid);
  uint64_t reg = READ_SYSREG(ELR_EL1);
  printf("ELR_EL1: %x\r\n", reg);
  reg = READ_SYSREG(SPSR_EL1);
  printf("SPSR_EL1: %x\r\n", reg);
  reg = READ_SYSREG(ESR_EL1);
  printf("ESR_EL1: %x\r\n\r\n", reg);
}

void default_exception_handler() { print_exception_imformation(NULL); }

void _el1_lower_el_aarch64_sync_handler(trapframe_t *tf) {
  // ENABLE_IRQ();
  // print_exception_imformation(tf);
  // DISABLE_IRQ();
  task_struct_t *current = get_current();
  current->trapframe = tf;
  uint64_t esr_el1 = READ_SYSREG(ESR_EL1);
  switch (esr_el1) {
  case 0x56000000: {
    if (syscall_handler(tf) == -1) {
      printf("Unknown syscall: %d\r\n", tf->gpr[8]);
    }
    break;
  }
  default:
    print_exception_imformation(tf);
  }
}

void _el1_lower_el_aarch64_irq_handler(trapframe_t *tf) {
  task_struct_t *current = get_current();
  current->trapframe = tf;
  irq_handler_entry();
}

void _el1_lower_el_aarch64_fiq_handler() {}

void _el1_lower_el_aarch64_serror_handler() {}

void _el1_current_el_aarch64_sync_handler() {
  print_exception_imformation(NULL);
}

void _el1_current_el_aarch64_irq_handler() { irq_handler_entry(); }

void _el1_current_el_aarch64_fiq_handler() {}

void _el1_current_el_aarch64_serror_handler() {}
