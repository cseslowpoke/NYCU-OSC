#ifndef __SCHED_H
#define __SCHED_H

#include "common/list.h"
#include "core/task.h"

void sched_init(void);

void sched(void);

void sched_add(task_struct_t *task);

void sched_idle(void);

void sched_kill_task(uint32_t pid);

#endif // __SCHED_H
