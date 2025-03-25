#ifndef __COMMAND_H
#define __COMMAND_H

#include "common/types.h"

// typdef for command function
typedef void (*command_func_t)(uint32_t argc, const char *argv[]);

typedef struct {
  char *name;
  command_func_t func;
} command_t;

#define COMMAND_FUNC(name) void cmd_##name(uint32_t argc, const char *argv[])

COMMAND_FUNC(help);
COMMAND_FUNC(hello);
COMMAND_FUNC(mailbox);
COMMAND_FUNC(reboot);
COMMAND_FUNC(ls);
COMMAND_FUNC(cat);
COMMAND_FUNC(mem_alloc);
COMMAND_FUNC(exec);
COMMAND_FUNC(set_timeout);

#endif
