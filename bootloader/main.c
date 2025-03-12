#include "uart.h"
#include "utils.h"

#define KERNEL_BASE_ADDRESS ((volatile unsigned char *)(0x80000))
#define KERNEL_MAGIC 0x544F4F42

int main(void *dtb_addr) {
  uart_init();
  while (1) {
    uart_send_string("UART bootloader: Waiting for kernel...\r\n");
    unsigned int boot, length, checksum;
    uart_recv_bytes((unsigned char *)&boot, 4);
    if (boot == KERNEL_MAGIC) {
      uart_recv_bytes((unsigned char *)&length, 4);
      uart_recv_bytes((unsigned char *)&checksum, 4);
      volatile unsigned char *des = KERNEL_BASE_ADDRESS;
      uart_recv_bytes((unsigned char *)des, length);
      unsigned int check = 0;
      for (int i = 0; i < length; i++) {
        check += des[i];
      }
      if (check != checksum) {
        continue;
      } else {
      }
      __asm__ volatile("mov x0, %0" : : "r"(dtb_addr) : "x0");
      __asm__ volatile("ldr x1, =0x80000");
      __asm__ volatile("br x1");
    }
  }
  return 0;
}
