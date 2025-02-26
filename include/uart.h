#define PBASE 0x3f000000

void uart_init();

void uart_send(char c);

char uart_recv();

void uart_send_string(char *str);
