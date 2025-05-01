#ifndef __SCHED_H
#define __SCHED_H

#include "common/list.h"
#include "core/task.h"

void sched_init(void);

void sched(void);

void sched_add(task_struct_t *task);

void sched_idle(void);

void sched_kill_task(uint32_t pid);

void switch_to(task_struct_t *prev, task_struct_t *next);
#endif // __SCHED_H
