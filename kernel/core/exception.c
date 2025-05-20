#include "core/exception.h"
#include "common/printf.h"
#include "common/types.h"
#include "common/utils.h"
#include "core/syscall.h"
#include "core/task.h"
#include "drivers/irq.h"
#include "mm/mmu.h"

void print_exception_imformation(trapframe_t *tf) {
  // task_struct_t *current = get_current();
  // printf("Exception in task pid %d\r\n", current->pid);
  uint64_t reg = READ_SYSREG(ELR_EL1);
  debug_printf("ELR_EL1: 0x%x\r\n", reg);
  reg = READ_SYSREG(SPSR_EL1);
  debug_printf("SPSR_EL1: 0x%x\r\n", reg);
  reg = READ_SYSREG(ESR_EL1);
  debug_printf("ESR_EL1: 0x%x\r\n", reg);
  reg = READ_SYSREG(FAR_EL1);
  debug_printf("FAR_EL1: 0x%x\r\n", reg);
}

void default_exception_handler() { print_exception_imformation(NULL); }

#define SVC 0b010101
#define DataAbort_El0 0b100100
#define DataAbort_El1 0b100101
#define InstAbort_El0 0b100000
#define InstAbort_El1 0b100001

#define TranslationFault 0b0001
#define AccessFlagFault 0b0010
#define PermissionFault 0b0011

static void handle_data_inst_abort(uint64_t esr_el1, trapframe_t *tf) {
  uint64_t far_el1 = READ_SYSREG(FAR_EL1);
  uint64_t sc = (esr_el1 >> 2) & 0xf;
  switch (sc) {
  case TranslationFault:
    do_page_fault(far_el1, get_current());
    break;
  case PermissionFault:
    do_permission_fault(far_el1, get_current());
    break;
  case AccessFlagFault:
  default:
    debug_printf("Data/Instruction Abort at %x\r\n", far_el1);
    print_exception_imformation(tf);
    while (1)
      ;
  }
}

void _el1_lower_el_aarch64_sync_handler(trapframe_t *tf) {
  DISABLE_IRQ();
  task_struct_t *current = get_current();
  current->trapframe = tf;
  uint64_t esr_el1 = READ_SYSREG(ESR_EL1);
  uint64_t esr_ec = (esr_el1 >> 26) & 0x3f;
  switch (esr_ec) {
  case SVC: {
    if (syscall_handler(tf) == -1) {
      debug_printf("Unknown syscall: %x\r\n", tf->gpr[8]);
      print_exception_imformation(tf);
    }
    break;
  }
  case DataAbort_El0: {
    handle_data_inst_abort(esr_el1, tf);
    break;
  }
  case InstAbort_El0: {
    handle_data_inst_abort(esr_el1, tf);
    break;
  }
  default:
    print_exception_imformation(tf);
    ENABLE_IRQ();
    while (1)
      ;
  }
}

void _el1_lower_el_aarch64_irq_handler(trapframe_t *tf) {
  // debug_printf("IRQ\r\n");
  task_struct_t *current = get_current();
  current->trapframe = tf;
  irq_handler_entry();
}

void _el1_lower_el_aarch64_fiq_handler() {}

void _el1_lower_el_aarch64_serror_handler() {}

void _el1_current_el_aarch64_sync_handler() {
  DISABLE_IRQ();
  task_struct_t *current = get_current();
  uint64_t esr_el1 = READ_SYSREG(ESR_EL1);
  uint64_t esr_ec = (esr_el1 >> 26) & 0x3f;
  trapframe_t *tf = current->trapframe;
  switch (esr_ec) {
  case SVC: {
    if (syscall_handler(tf) == -1) {
      debug_printf("Unknown syscall: %x\r\n", tf->gpr[8]);
      print_exception_imformation(tf);
    }
    break;
  }
  case DataAbort_El1: {
    handle_data_inst_abort(esr_el1, tf);
    break;
  }
  case InstAbort_El1: {
    handle_data_inst_abort(esr_el1, tf);
    break;
  }
  default:
    print_exception_imformation(tf);
    ENABLE_IRQ();
    while (1)
      ;
  }
}

void _el1_current_el_aarch64_irq_handler() { irq_handler_entry(); }

void _el1_current_el_aarch64_fiq_handler() {}

void _el1_current_el_aarch64_serror_handler() {}
