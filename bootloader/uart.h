#ifndef __UART_H
#define __UART_H
#define GPFSEL1 ((volatile unsigned int *)0x3f200004)
#define GPPUD ((volatile unsigned int *)0x3f200094)
#define GPPUDCLK0 ((volatile unsigned int *)0x3f200098)

#define AUX_ENABLES ((volatile unsigned int *)0x3f215004)
#define AUX_MU_CNTL ((volatile unsigned int *)0x3f215060)
#define AUX_MU_IER ((volatile unsigned int *)0x3f215044)
#define AUX_MU_LCR ((volatile unsigned int *)0x3f21504c)
#define AUX_MU_MCR ((volatile unsigned int *)0x3f215050)
#define AUX_MU_BAUD ((volatile unsigned int *)0x3f215068)
#define AUX_MU_IIR ((volatile unsigned int *)0x3f215048)
#define AUX_MU_LSR ((volatile unsigned int *)0x3f215054)
#define AUX_MU_IO ((volatile unsigned int *)0x3f215040)

#define UART_CAN_READ() ((*AUX_MU_LSR) & 0x01)
#define UART_CAN_WRITE() ((*AUX_MU_LSR) & 0x20)

#define DELAY_CYCLES(cycles) do {              \
    int _c = (cycles);                           \
    while (_c-- > 0) {                           \
      __asm__ volatile("nop");                   \
    }                                            \
  } while (0)
  
void uart_init();

void uart_send(char c);

char uart_recv();

void uart_send_string(const char *str);

void uart_recv_bytes(unsigned char *buf, unsigned int size);


#endif // __UART_H
