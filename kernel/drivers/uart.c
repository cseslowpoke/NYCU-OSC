#include "drivers/uart.h"
#include "common/utils.h"
#include "drivers/irq.h"

volatile uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_rx_head, uart_rx_tail;

volatile uint8_t uart_tx_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_tx_head, uart_tx_tail;

void uart_init() {
  // setting gpio 14 and 15 to alternative function 5 (mini uart)
  *GPFSEL1 = 0x12000;

  // setting gpio Pull-up/down control register
  *GPPUD = 0;
  DELAY_CYCLES(150);
  *GPPUDCLK0 = (1 << 14) | (1 << 15);
  DELAY_CYCLES(150);
  *GPPUD = 0;
  *GPPUDCLK0 = 0;

  // setting mini uart registers
  *AUX_ENABLE = 1u;
  *AUX_MU_CNTL = 0u;
  *AUX_MU_IER = 1u;
  *AUX_MU_LCR = 3u;
  *AUX_MU_MCR = 0;
  *AUX_MU_BAUD = 270;
  *AUX_MU_IIR = 0x6;

  *AUX_MU_CNTL = 3;

  irq_enable(AUX_IRQ_NUM);
  irq_register(AUX_IRQ_NUM, uart_irq_handler);
}

#ifdef POLLING
void uart_send(char c) {
  while (!(*AUX_MU_LSR & 0x20))
    ;
  *AUX_MU_IO = c;
}

char uart_recv() {
  while (!(*AUX_MU_LSR & 0x1))
    ;
  return (uint8_t)*AUX_MU_IO;
}
#else
void uart_send(char c) {
  uart_tx_buffer[uart_tx_head++] = c;
  uart_tx_head %= UART_BUFFER_SIZE;
  *AUX_MU_IER |= 0x2;
}
char uart_recv() {
  while (uart_rx_head == uart_rx_tail)
    ;
  return uart_rx_buffer[uart_rx_tail++];
}
#endif

void uart_send_string(const char *str) {
  while (*str) {
    uart_send(*str++);
  }
}

void uart_recv_bytes(unsigned char *buf, unsigned int size) {
  for (int i = 0; i < size; i++) {
    buf[i] = uart_recv();
  }
}

void uart_irq_handler() {
  irq_disable(AUX_IRQ_NUM);
  volatile unsigned int status = *AUX_MU_IIR;

  if (status & 0x1) {
    return;
  }
  switch (status & 0x6) {
  case 0x2: {
    *AUX_MU_IO = uart_tx_buffer[uart_tx_tail++];
    uart_tx_tail %= UART_BUFFER_SIZE;
    if (uart_tx_head == uart_tx_tail) {
      *AUX_MU_IER &= ~0x2;
    }
    break;
  }
  case 0x4: {
    uart_rx_buffer[uart_rx_head++] = (uint8_t)*AUX_MU_IO;
    uart_rx_head %= UART_BUFFER_SIZE;
    break;
  }
  default: {
  }
  }
  irq_enable(AUX_IRQ_NUM);
}
