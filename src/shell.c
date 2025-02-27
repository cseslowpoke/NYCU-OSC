#include "shell.h"
#include "uart.h"

static void cmd_help() {
  uart_send_string("help     : print this help menu\r\n"
                   "hello    : print Hello World!\r\n"
                   "mailbox  : print hardware's information (not implement)\r\n"
                   "reboot   : reboot the device (not implement)\r\n");
}

static void cmd_hello() { uart_send_string("Hello, world!\r\n"); }

command_t command[] = {{"help", cmd_help}, {"hello", cmd_hello}, {0, 0}};

void shell_start() {
  char s[] __attribute__((aligned(8))) = "Hello, world!\r\n";
  uart_send_string(s);
  char buf[SHELL_BUFFER_SIZE] = {};
  int buf_len = 0;
  uart_send_string("# ");
  while (1) {
    char c = uart_recv();
    if (c == '\r' || c == '\n') {
      uart_send('\r');
      uart_send('\n');
      buf[buf_len] = '\0';
      shell_process_command(buf, buf_len);
      buf_len = 0;
      uart_send_string("# ");
    } else if (c == '\b' || c == 127) {
      if (buf_len > 0) {
        buf_len--;
        uart_send_string("\b \b");
      }
    } else if (buf_len < SHELL_BUFFER_SIZE - 1) {
      buf[buf_len++] = c;
      uart_send(c);
    }
  }
}

void shell_process_command(char *buf, int buf_len) {
  int iter = 0;
  while (command[iter].name != 0) {
    int flag = 0;
    for (int i = 0; i < buf_len; i++) {
      if (buf[i] == '\0' && command[iter].name[i] == '\0') {
        break;
      }
      if (buf[i] != command[iter].name[i]) {
        flag = 1;
        break;
      }
    }
    if (flag) {
      iter++;
      continue;
    }
    command[iter].func();
    return;
  }
  uart_send_string("Command not found\r\n");
}
