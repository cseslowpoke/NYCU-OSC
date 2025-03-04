#include "shell.h"
#include "mailbox.h"
#include "mbox_tags.h"
#include "uart.h"

static void cmd_help() {
  uart_send_string("help     : print this help menu\r\n"
                   "hello    : print Hello World!\r\n"
                   "mailbox  : print hardware's information\r\n"
                   "reboot   : reboot the device (not implement)\r\n");
}

static void cmd_hello() { uart_send_string("Hello, world!\r\n"); }

int get_board_revision() {
  mailbox[0] = 7 * 4;
  mailbox[1] = MBOX_REQUEST;
  mailbox[2] = MBOX_TAG_GET_BOARD_REVISION;
  mailbox[3] = 4;
  mailbox[4] = MBOX_TAG_REQUEST;
  mailbox[5] = 0;
  mailbox[6] = MBOX_TAG_LAST;

  if (mailbox_call(MBOX_CH_PROP)) {
    return mailbox[5];
  } else {
    return -1;
  }
}

typedef struct __attribute__((aligned(8))) {
  unsigned int base;
  unsigned int size;
} arm_memory_t;

arm_memory_t get_arm_memory() {
  mailbox[0] = 8 * 4;
  mailbox[1] = MBOX_REQUEST;
  mailbox[2] = MBOX_TAG_GET_ARM_MEMORY;
  mailbox[3] = 8;
  mailbox[4] = MBOX_TAG_REQUEST;
  mailbox[5] = 0;
  mailbox[6] = 0;
  mailbox[7] = MBOX_TAG_LAST;

  arm_memory_t res;
  if (mailbox_call(MBOX_CH_PROP)) {
    res.base = mailbox[5];
    res.size = mailbox[6];
  } else {
    res.base = 0;
    res.size = 0;
  }
  return res;
}

void u32tohex(unsigned int val, char *buf) {
  for (int i = 0; i < 8; i++) {
    int nibble = (val >> (28 - i * 4)) & 0xF;
    if (nibble < 10) {
      buf[i] = '0' + nibble;
    } else {
      buf[i] = 'A' + nibble - 10;
    }
  }
  buf[8] = '\0';
}

static void cmd_mailbox() {
  char hexstr_buf[9] = {};
  uart_send_string("Mailbox info:\r\n");
  // Board information
  unsigned int board_revision = get_board_revision();
  uart_send_string("Board revision: 0x");
  u32tohex(board_revision, hexstr_buf);
  uart_send_string(hexstr_buf);
  uart_send_string("\r\n");
  // Arm memory information
  arm_memory_t arm_memory = get_arm_memory();
  uart_send_string("Arm memory base address: 0x");
  u32tohex(arm_memory.base, hexstr_buf);
  uart_send_string(hexstr_buf);
  uart_send_string("\r\n");
  uart_send_string("Arm memory size: 0x");
  u32tohex(arm_memory.size, hexstr_buf);
  uart_send_string(hexstr_buf);
  uart_send_string("\r\n");
}

static command_t command[] = {
    {"help", cmd_help}, {"hello", cmd_hello}, {"mailbox", cmd_mailbox}, {0, 0}};

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
  uart_send_string("command not found: ");
  uart_send_string(buf);
  uart_send_string("\r\n");
}
