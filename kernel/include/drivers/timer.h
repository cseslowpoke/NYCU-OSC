#ifndef __TIMER_H
#define __TIMER_H

#include "common/list.h"
#include "common/types.h"
#include "common/utils.h"

#define CORE0_TIMER_IRQ_CTRL ((volatile uint32_t *)(VM_BASE + 0x40000040))

typedef void (*timer_handler_t)(void *);

/* struct timer_task - a timer task
 * @func: the handler to be called when the timer expires
 * @arg: the argument to be passed to the handler
 * @time: the time in milliseconds
 * @next: the next timer task
 */
typedef struct timer_task {
  timer_handler_t func;
  void *arg;
  uint64_t time;
  list_head_t list;
} timer_task_t;

void timer_init(void);

void timer_irq_handler(void);

void timer_irq_task(void);

void timer_set(uint64_t time);

// add a timer event that triggers after a specified time.
void timer_add_task(timer_handler_t handler, void *arg, uint32_t time);

// list of timer tasks
extern list_head_t timer_tasks;

// wrapper for lab3 basic2

void timer_lab3_basic2_task();

#endif // __TIMER_H
