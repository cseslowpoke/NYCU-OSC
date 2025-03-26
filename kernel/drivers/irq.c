#include "drivers/irq.h"
#include "common/list.h"
#include "common/utils.h"
#include "core/simple_alloc.h"
#include "drivers/timer.h"

irq_hadler_t irq_table[MAX_IRQ];

void irq_init() {
  for (int i = 0; i < MAX_IRQ; i++) {
    irq_table[i] = NULL;
  }
  INIT_LIST_HEAD(&irq_task_queue);
  ENABLE_IRQ();
}

void irq_register(int irq_num, irq_hadler_t handler) {
  if (irq_num < 0 || irq_num >= MAX_IRQ) {
    return;
  }
  irq_table[irq_num] = handler;
}

void irq_enable(int irq_num) {
  if (irq_num < 0 || irq_num >= MAX_IRQ) {
    return;
  }
  if (irq_num < 32) {
    *IRQ_ENABLE1_REG = 1 << irq_num;
  } else {
    *IRQ_ENABLE2_REG = 1 << (irq_num - 32);
  }
}

void irq_disable(int irq_num) {
  if (irq_num < 0 || irq_num >= MAX_IRQ) {
    return;
  }
  if (irq_num < 32) {
    *IRQ_DISABLE1_REG = 1 << irq_num;
  } else {
    *IRQ_DISABLE2_REG = 1 << (irq_num - 32);
  }
}

void irq_handler_entry() {

  uint32_t irq = *IRQ_PENDING1_REG;
  for (int i = 0; i < 32; i++) {
    if ((irq >> i) & 0x1) {
      if (irq_table[i] != NULL) {
        irq_table[i]();
        goto exit;
      }
    }
  }
  irq = *IRQ_PENDING2_REG;
  for (int i = 0; i < 32; i++) {
    if ((irq >> i) & 0x1) {
      if (irq_table[i + 32] != NULL) {
        irq_table[i + 32]();
        goto exit;
      }
    }
  }
  timer_irq_handler();
exit:
  irq_task_exec();
  return;
}

list_head_t irq_task_queue;

void irq_task_enqueue(int priority, irq_task_handler_t handler) {
  DISABLE_IRQ();
  irq_task_t *task = (irq_task_t *)simple_alloc(sizeof(irq_task_t));
  // NOTE: Should handle allocation failure

  task->priority = priority;
  task->handler = handler;
  list_head_t *pos;
  list_for_each(pos, &irq_task_queue) {
    irq_task_t *t = container_of(pos, irq_task_t, list);
    if (t->priority > priority) {
      list_add_tail(&task->list, pos);
      ENABLE_IRQ();
      return;
    }
  }
  list_add_tail(&task->list, &irq_task_queue);
  ENABLE_IRQ();
}

irq_task_handler_t irq_task_dequeue() {
  DISABLE_IRQ();
  if (list_empty(&irq_task_queue)) {
    return NULL;
  }
  list_head_t *pos = irq_task_queue.next;
  irq_task_t *task = container_of(pos, irq_task_t, list);
  list_del(pos);
  irq_task_handler_t handler = task->handler;

  // NOTE: Should handle deallocation(free)

  ENABLE_IRQ();
  return handler;
}

void irq_task_exec() {
  irq_task_handler_t handler;
  while (1) {
    handler = irq_task_dequeue();
    if (handler == NULL) { // when no task in the queue
      break;
    }
    handler();
  }
}
