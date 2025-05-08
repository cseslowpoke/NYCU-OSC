#ifndef __MAILBOX_H
#define __MAILBOX_H

#include "common/utils.h"

#define MAILBOX_BASE (MMIO_BASE + 0xb880)

#define MAILBOX_READ ((volatile unsigned int *)(MAILBOX_BASE + 0x00))
#define MAILBOX_STATUS ((volatile unsigned int *)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE ((volatile unsigned int *)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY 0x40000000
#define MAILBOX_FULL 0x80000000

#define MBOX_REQUEST 0x00000000

#define MBOX_CH_PROP 8

extern unsigned int mailbox[36] __attribute__((aligned(16)));

int mailbox_call(char channel);

int mailbox_call_with_mail(char ch, unsigned int *mbox);
#endif // __MAILBOX_H
