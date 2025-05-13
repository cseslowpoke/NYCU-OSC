#ifndef __USER_EXEC_H
#define __USER_EXEC_H
#include "common/types.h"

#define USER_SPACE_BEGIN 0x0
#define USER_STACK_BEGIN 0xffffffffb000
#define USER_STACK_END 0xfffffffff000

#define PERIPHERAL_BEGIN 0x3c000000
#define PERIPHERAL_END 0x3f000000

void user_exec(void *prog, uint32_t prog_len);

int32_t do_exec(const char *filename, char *const argv[]);

#endif
