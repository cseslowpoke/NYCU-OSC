#include "core/shell.h"
#include "common/printf.h"
#include "common/string.h"
#include "common/types.h"
#include "core/command.h"
#include "drivers/uart.h"

static command_t command[] = {{"help", cmd_help},
                              {"hello", cmd_hello},
                              {"mailbox", cmd_mailbox},
                              {"reboot", cmd_reboot},
                              {"ls", cmd_ls},
                              {"cat", cmd_cat},
                              {"mem_alloc", cmd_mem_alloc},
                              {"exec", cmd_exec},
                              {"set_timeout", cmd_set_timeout},
                              {0, 0}};

void shell_start() {
  printf("Welcome to rpi3b+\r\n");
  char buf[SHELL_BUFFER_SIZE] = {};
  int buf_len = 0;
  printf("# ");
  while (1) {
    char c = uart_recv();
    if (c == '\r' || c == '\n') {
      printf("\r\n");
      buf[buf_len] = '\0';
      shell_process_command(buf, buf_len);
      buf_len = 0;
      printf("# ");
    } else if (c == '\b' || c == 127) {
      if (buf_len > 0) {
        buf_len--;
        printf("\b \b");
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
    command[iter].func(argc, (const char **)argv);
    return;
  }
  printf("command not found: %s\r\n", argv[0]);
}
