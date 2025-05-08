#ifndef __UART_H
#define __UART_H

#include "common/types.h"
#include "common/utils.h"

#define GPFSEL1 ((volatile unsigned int *)(MMIO_BASE + 0x200004))
#define GPPUD ((volatile unsigned int *)(MMIO_BASE + 0x200094))
#define GPPUDCLK0 ((volatile unsigned int *)(MMIO_BASE + 0x200098))

#define AUX_ENABLE ((volatile unsigned int *)(MMIO_BASE + 0x215004))
#define AUX_MU_CNTL ((volatile unsigned int *)(MMIO_BASE + 0x215060))
#define AUX_MU_IER ((volatile unsigned int *)(MMIO_BASE + 0x215044))
#define AUX_MU_LCR ((volatile unsigned int *)(MMIO_BASE + 0x21504c))
#define AUX_MU_MCR ((volatile unsigned int *)(MMIO_BASE + 0x215050))
#define AUX_MU_BAUD ((volatile unsigned int *)(MMIO_BASE + 0x215068))
#define AUX_MU_IIR ((volatile unsigned int *)(MMIO_BASE + 0x215048))
#define AUX_MU_LSR ((volatile unsigned int *)(MMIO_BASE + 0x215054))
#define AUX_MU_IO ((volatile unsigned int *)(MMIO_BASE + 0x215040))
#define AUX_IRQ_NUM 29

#define UART_CAN_READ() ((*AUX_MU_LSR) & 0x01)
#define UART_CAN_WRITE() ((*AUX_MU_LSR) & 0x20)
#define UART_BUFFER_SIZE 8192

void uart_init();

void uart_send(char c);

char uart_recv();

void uart_send_string(const char *str);

uint32_t uart_recv_bytes(unsigned char *buf, unsigned int size);

void uart_irq_handler();

void uart_irq_task();

#endif // __UART_H
