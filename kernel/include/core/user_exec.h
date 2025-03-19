#ifndef __USER_EXEC_H
#define __USER_EXEC_H
#include "common/types.h"

#define USER_SPACE_BEGIN 0x00800000
#define USER_SPACE_END 0x00A00000

#define USER_STACK_BEGIN 0x00A00000
#define USER_STACK_END 0x00B00000

void user_exec(void *prog, uint32_t prog_len);

#endif
