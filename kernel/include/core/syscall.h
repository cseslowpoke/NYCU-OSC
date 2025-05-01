#ifndef __SYSCALL_H
#define __SYSCALL_H

/*
 * syscall.h
 */

#include "common/types.h"
#include "core/task.h"

#define SYSCALL_MAX 10

#define SYSCALL_DEF(num, name, ret_type, args) ret_type sys_##name args;

#include "core/syscall_table.h"

#undef SYSCALL_DEF

int32_t syscall_handler(trapframe_t *tf);

#endif // __SYSCALL_H
