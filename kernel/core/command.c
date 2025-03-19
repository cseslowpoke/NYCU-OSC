#include "fs/file.h"
#include "drivers/mailbox.h"
#include "drivers/mbox_tags.h"
#include "core/simple_alloc.h"
#include "common/types.h"
#include "drivers/uart.h"
#include "core/user_exec.h"
#include "common/utils.h"
#include "drivers/watchdog.h"

void cmd_help(unsigned int argc, const char *argv[]) {
  uart_send_string("help      : print this help menu\r\n"
                   "hello     : print Hello World!\r\n"
                   "mailbox   : print hardware's information\r\n"
                   "reboot    : reboot the device\r\n"
                   "ls        : list files in the initramfs\r\n"
                   "cat       : print the content of a file\r\n"
                   "mem_alloc : allocate memory\r\n");
}

void cmd_hello(unsigned int argc, const char *argv[]) {
  uart_send_string("Hello, world!\r\n");
}

static int get_board_revision() {
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

static arm_memory_t get_arm_memory() {
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

void cmd_mailbox(unsigned int argc, const char *argv[]) {
  char hexstr_buf[9] = {};
  uart_send_string("Mailbox info:\r\n");
  // Board information
  unsigned int board_revision = get_board_revision();
  uart_send_string("Board revision: 0x");
  uint2hex(board_revision, hexstr_buf);
  uart_send_string(hexstr_buf);
  uart_send_string("\r\n");
  // Arm memory information
  arm_memory_t arm_memory = get_arm_memory();
  uart_send_string("Arm memory base address: 0x");
  uint2hex(arm_memory.base, hexstr_buf);
  uart_send_string(hexstr_buf);
  uart_send_string("\r\n");
  uart_send_string("Arm memory size: 0x");
  uint2hex(arm_memory.size, hexstr_buf);
  uart_send_string(hexstr_buf);
  uart_send_string("\r\n");
}

void cmd_reboot(unsigned int argc, const char *argv[]) {
  uart_send_string("reboot\r\n");
  reset(100);
  while (1)
    ;
}

static void cmd_ls_append_ln(const char *filename) {
  uart_send_string(filename);
  uart_send_string("\r\n");
}

void cmd_ls(unsigned int argc, const char *argv[]) {
  file_list(cmd_ls_append_ln);
}

void cmd_cat(unsigned int argc, const char *argv[]) {
  if (argc != 2) {
    uart_send_string("Usage: cat <filename>\r\n");
    return;
  }
  char *buf;
  int size = file_find(argv[1], &buf);
  if (size == -1) {
    uart_send_string("File not found: \r\n");
    return;
  }
  while (size--) {
    if (*buf == '\n') {
      uart_send('\r');
    }
    uart_send(*buf++);
  }
  uart_send_string("\r\n");
}

void cmd_mem_alloc(unsigned int argc, const char *argv[]) {
  if (argc != 2) {
    uart_send_string("Usage: mem_alloc <size>\r\n");
    return;
  }
  int size = atoi(argv[1]);
  if (size <= 0) {
    uart_send_string("Invalid size\r\n");
    return;
  }
  void *ptr = simple_alloc(size);
  if (ptr == NULL) {
    uart_send_string("Failed to allocate memory\r\n");
    return;
  }
  uart_send_string("Allocated memory at: 0x");
  char hexstr_buf[9] = {};
  uint2hex((unsigned int)ptr, hexstr_buf);
  uart_send_string(hexstr_buf);
  uart_send_string("\r\n");
}

void cmd_exec(unsigned int argc, const char *argv[]) {
  if (argc != 2) {
    uart_send_string("Usage: user_exec <filename>\r\n");
    return;
  }
  char *buf;
  int size = file_find(argv[1], &buf);
  if (size == -1) {
    uart_send_string("File not found: \r\n");
    return;
  }
  user_exec(buf, size);
}
