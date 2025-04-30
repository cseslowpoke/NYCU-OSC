#include "common/printf.h"
#include "common/string.h"
#include "common/types.h"
#include "common/utils.h"
#include "core/exec.h"
#include "drivers/mailbox.h"
#include "drivers/mbox_tags.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "drivers/watchdog.h"
#include "fs/file.h"
#include "mm/simple_alloc.h"
#include "mm/slab.h"

void cmd_help(unsigned int argc, const char *argv[]) {
  printf("help      : print this help menu\r\n"
         "hello     : print Hello World!\r\n"
         "mailbox   : print hardware's information\r\n"
         "reboot    : reboot the device\r\n"
         "ls        : list files in the initramfs\r\n"
         "cat       : print the content of a file\r\n"
         "mem_alloc : allocate memory\r\n");
}

void cmd_hello(unsigned int argc, const char *argv[]) {
  printf("Hello, world!\r\n");
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
  printf("Mailbox info:\r\n");
  // Board information
  unsigned int board_revision = get_board_revision();
  printf("Board revision: 0x%x\r\n", board_revision);
  // Arm memory information
  arm_memory_t arm_memory = get_arm_memory();
  printf("Arm memory base address: 0x%x\r\n", arm_memory.base);
  printf("Arm memory size: 0x%x\r\n", arm_memory.size);
}

void cmd_reboot(unsigned int argc, const char *argv[]) {
  printf("Rebooting...\r\n");
  reset(100);
  while (1)
    ;
}

static void cmd_ls_append_ln(const char *filename) {
  printf("%s\r\n", filename);
}

void cmd_ls(unsigned int argc, const char *argv[]) {
  file_list(cmd_ls_append_ln);
}

void cmd_cat(unsigned int argc, const char *argv[]) {
  if (argc != 2) {
    printf("Usage: cat <filename>\r\n");
    return;
  }
  char *buf;
  int size = file_find(argv[1], &buf);
  if (size == -1) {
    printf("File not found: \r\n");
    return;
  }
  while (size--) {
    if (*buf == '\n') {
      uart_send('\r');
    }
    uart_send(*buf++);
  }
  printf("\r\n");
}

void cmd_mem_alloc(unsigned int argc, const char *argv[]) {
  if (argc != 2) {
    printf("Usage: mem_alloc <size>\r\n");
    return;
  }
  int size = atoi(argv[1]);
  if (size <= 0) {
    printf("Invalid size\r\n");
    return;
  }
  void *ptr = simple_alloc(size);
  if (ptr == NULL) {
    printf("Failed to allocate memory\r\n");
    return;
  }
  printf("Allocated Memory at 0x%x\r\n", (uint64_t)ptr);
}

void cmd_exec(unsigned int argc, const char *argv[]) {
  if (argc != 2) {
    printf("Usage: user_exec <filename>\r\n");
    return;
  }
  do_exec(argv[1], NULL);
}

void cmd_set_timeout(unsigned int argc, const char *argv[]) {
  if (argc != 3) {
    printf("Usage: set_timeout <message> <seconds>\r\n");
    return;
  }
  int time = atoi(argv[2]);
  if (time <= 0) {
    printf("Invalid time\r\n");
    return;
  }
  char *msg = (char *)kmalloc(strlen(argv[1]) + 3);
  strcpy(msg, argv[1]);
  msg[strlen(argv[1])] = '\r';
  msg[strlen(argv[1]) + 1] = '\n';
  msg[strlen(argv[1]) + 2] = '\0';

  timer_add_task((timer_handler_t)uart_send_string, (void *)msg,
                 time * READ_SYSREG(CNTFRQ_EL0));
}
