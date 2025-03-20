#include "core/exception.h"
#include "common/types.h"
#include "common/utils.h"
#include "drivers/uart.h"

void default_exception_handler() {
  char buf[100];
  uint64_t reg = READ_SYSREG(ELR_EL1);
  uint2hex(reg, buf);
  uart_send_string("ELR_EL1: ");
  uart_send_string(buf);
  uart_send_string("\r\n");
  reg = READ_SYSREG(SPSR_EL1);
  uint2hex(reg, buf);
  uart_send_string("SPSR_EL1: ");
  uart_send_string(buf);
  uart_send_string("\r\n");
  reg = READ_SYSREG(ESR_EL1);
  uart_send_string("ESR_EL1: ");
  uart_send_string(buf);
  uart_send_string("\r\n\r\n");
}

void _el1_lower_el_aarch64_sync_handler() {
  char buf[100];
  uint64_t reg = READ_SYSREG(ELR_EL1);
  uint2hex(reg, buf);
  uart_send_string("ELR_EL1: ");
  uart_send_string(buf);
  uart_send_string("\r\n");
  reg = READ_SYSREG(SPSR_EL1);
  uint2hex(reg, buf);
  uart_send_string("SPSR_EL1: ");
  uart_send_string(buf);
  uart_send_string("\r\n");
  reg = READ_SYSREG(ESR_EL1);
  uart_send_string("ESR_EL1: ");
  uart_send_string(buf);
  uart_send_string("\r\n\r\n");
}

void _el1_lower_el_aarch64_irq_handler() {}

void _el1_lower_el_aarch64_fiq_handler() {}

void _el1_lower_el_aarch64_serror_handler() {}

void _el1_current_el_aarch64_sync_handler() {}

void _el1_current_el_aarch64_irq_handler() {}

void _el1_current_el_aarch64_fiq_handler() {}

void _el1_current_el_aarch64_serror_handler() {}
