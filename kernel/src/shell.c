#include "shell.h"
#include "command.h"
#include "string.h"
#include "types.h"
#include "uart.h"

static command_t command[] = {{"help", cmd_help},
                              {"hello", cmd_hello},
                              {"mailbox", cmd_mailbox},
                              {"reboot", cmd_reboot},
                              {"ls", cmd_ls},
                              {"cat", cmd_cat},
                              {"mem_alloc", cmd_mem_alloc},
                              {0, 0}};

void shell_start() {
  char s[] __attribute__((aligned(8))) = "Hello, world!\r\n";
  uart_send_string(s);
  char buf[SHELL_BUFFER_SIZE] = {};
  int buf_len = 0;
  uart_send_string("# ");
  while (1) {
    char c = uart_recv();
    if (c == '\r' || c == '\n') {
      uart_send_string("\r\n");
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
  char *argv[SHELL_MAX_ARGC] = {};
  int argc = 0;
  char *token = strtok(buf, ' ');
  while (token != NULL) {
    argv[argc++] = token;
    token = strtok(NULL, ' ');
  }

  argv[argc] = NULL;

  if (argc == 0) {
    return;
  }

  int iter = 0;
  while (command[iter].name != 0) {
    if (strcmp(argv[0], command[iter].name) != 0) {
      iter++;
      continue;
    }
    command[iter].func(argc, argv);
    return;
  }
  uart_send_string("command not found: ");
  uart_send_string(argv[0]);
  uart_send_string("\r\n");
}
