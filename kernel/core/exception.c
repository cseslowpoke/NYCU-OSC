#include "core/exception.h"
#include "common/printf.h"
#include "common/types.h"
#include "common/utils.h"
#include "drivers/irq.h"
#include "drivers/timer.h"
#include "drivers/uart.h"

void print_exception_imformation() {
  char buf[100];
  uint64_t reg = READ_SYSREG(ELR_EL1);
  printf("ELR_EL1: %x\r\n", reg);
  reg = READ_SYSREG(SPSR_EL1);
  printf("SPSR_EL1: %x\r\n", reg);
  reg = READ_SYSREG(ESR_EL1);
  printf("ESR_EL1: %x\r\n\r\n", reg);
}

void default_exception_handler() { print_exception_imformation(); }

void _el1_lower_el_aarch64_sync_handler() { print_exception_imformation(); }

void _el1_lower_el_aarch64_irq_handler() { irq_handler_entry(); }

void _el1_lower_el_aarch64_fiq_handler() {}

void _el1_lower_el_aarch64_serror_handler() {}

void _el1_current_el_aarch64_sync_handler() { print_exception_imformation(); }

void _el1_current_el_aarch64_irq_handler() { irq_handler_entry(); }

void _el1_current_el_aarch64_fiq_handler() {}

void _el1_current_el_aarch64_serror_handler() {}
