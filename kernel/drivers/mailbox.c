#include "drivers/mailbox.h"

unsigned int mailbox[36] __attribute__((aligned(16)));

int mailbox_call(char channel) {
  unsigned int msg =
      ((unsigned int)((unsigned long)&mailbox) & ~0xF) | (channel & 0xF);

  while (*MAILBOX_STATUS & MAILBOX_FULL)
    ;

  *MAILBOX_WRITE = msg;

  while (1) {
    while (*MAILBOX_STATUS & MAILBOX_EMPTY)
      ;

    if (*MAILBOX_READ == msg)
      return mailbox[1] == 0x80000000;
  }

  return 1;
}

int mailbox_call_with_mail(char ch, unsigned int *mbox) {
  for (int i = 0; i < 36; i++) {
    mailbox[i] = mbox[i];
  }
  unsigned int msg =
      ((unsigned int)((unsigned long)&mailbox) & ~0xF) | (ch & 0xF);

  while (*MAILBOX_STATUS & MAILBOX_FULL)
    ;

  *MAILBOX_WRITE = msg;

  while (1) {
    while (*MAILBOX_STATUS & MAILBOX_EMPTY)
      ;

    if (*MAILBOX_READ == msg) {
      for (int i = 0; i < 36; i++) {
        mbox[i] = mailbox[i];
      }
      return mailbox[1] == 0x80000000;
    }
  }

  return 1;
}
