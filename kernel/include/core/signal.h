#ifndef __SIGNAL_H
#define __SIGNAL_H
#include "common/types.h"

#define SIGN 32
typedef void (*sig_handler_t)(void *arg);

typedef struct signal_info {
  sig_handler_t handler[SIGN];
  uint32_t pending;
  void *stack;
} signal_info_t;

void signal_register(int signum, sig_handler_t handler);

void signal_send(int pid, int signum);

void signal_handler();

void signal_return();

#endif
