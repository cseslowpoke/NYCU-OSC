#define PBASE 0x3f000000
#define GPFSEL1 0x00200004
#define GPPUD 0x00200094
#define GPPUDCLK0 0x00200098

void uart_init();

void uart_send(char c);

char uart_recv();

void uart_send_string(char *str);
