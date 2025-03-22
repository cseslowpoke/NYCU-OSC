#include "drivers/timer.h"
#include "common/types.h"
#include "common/utils.h"
#include "drivers/uart.h"

void timer_init(void) {
  // Initialize the timer
  WRITE_SYSREG(CNTP_CTL_EL0, 1ll);
  WRITE_SYSREG(CNTP_TVAL_EL0, 2 * READ_SYSREG(CNTFRQ_EL0));

  *CORE0_TIMER_IRQ_CTRL = 2;
}

void timer_enable(void) {
  // Enable the timer
}

void timer_irq_handler(void) {

  int second = READ_SYSREG(CNTPCT_EL0) / READ_SYSREG(CNTFRQ_EL0);
  char buf[100];
  uint2hex(second, buf);
  uart_send_string("Timer interrupt: ");
  uart_send_string(buf);
  uart_send_string("\r\n");

  WRITE_SYSREG(CNTP_TVAL_EL0, 2 * READ_SYSREG(CNTFRQ_EL0));
  // Timer interrupt handler
}
