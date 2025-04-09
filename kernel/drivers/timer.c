#include "drivers/timer.h"
#include "common/list.h"
#include "common/printf.h"
#include "common/types.h"
#include "common/utils.h"
#include "core/exception.h"
#include "core/simple_alloc.h"
#include "drivers/irq.h"
#include "drivers/uart.h"

list_head_t timer_tasks;

void timer_init(void) {
  // Initialize the timer
  INIT_LIST_HEAD(&timer_tasks);
  *CORE0_TIMER_IRQ_CTRL = 2;
}

void timer_irq_handler(void) {
  WRITE_SYSREG(CNTP_CTL_EL0, 0ll);
  irq_task_enqueue(1, timer_irq_task);
}

void timer_irq_task(void) {
  list_head_t *event = timer_tasks.next;
  list_del(event);
  timer_task_t *task = container_of(event, timer_task_t, list);
  task->func(task->arg);
  if (!list_empty(&timer_tasks)) {
    timer_task_t *next = container_of(timer_tasks.next, timer_task_t, list);
    timer_set(next->time - READ_SYSREG(CNTPCT_EL0));
    WRITE_SYSREG(CNTP_CTL_EL0, 1ll);
  } else {
    WRITE_SYSREG(CNTP_CTL_EL0, 0ll);
  }

  // NOTE: should free event after implement complex allocator
}

void timer_set(uint64_t time) { WRITE_SYSREG(CNTP_TVAL_EL0, time); }

void timer_add_task(timer_handler_t handler, void *arg, uint32_t time) {
  // Add a timer event that triggers after a specified time
  timer_task_t *task = (timer_task_t *)simple_alloc(sizeof(timer_task_t));
  task->func = handler;
  task->arg = arg;
  task->time = READ_SYSREG(CNTPCT_EL0) + time * READ_SYSREG(CNTFRQ_EL0);

  if (list_empty(&timer_tasks)) {
    list_add_tail(&task->list, &timer_tasks);
    timer_set(time * READ_SYSREG(CNTFRQ_EL0));
    WRITE_SYSREG(CNTP_CTL_EL0, 1ll);
    return;
  }

  list_head_t *pos;
  list_for_each(pos, &timer_tasks) {
    timer_task_t *t = container_of(pos, timer_task_t, list);
    if (t->time > task->time) {
      list_add_tail(&task->list, pos);
      if (&task->list == timer_tasks.next) {
        timer_set(time * READ_SYSREG(CNTFRQ_EL0));
        WRITE_SYSREG(CNTP_CTL_EL0, 1ll);
      }
      return;
    }
  }
  list_add_tail(&task->list, &timer_tasks);
}

void timer_lab3_basic2_task() {
  int time = READ_SYSREG(CNTPCT_EL0);
  int frq = READ_SYSREG(CNTFRQ_EL0);
  printf("Current time: %d\r\n", time / frq);
  timer_add_task((timer_handler_t)timer_lab3_basic2_task, NULL, 2);
}
