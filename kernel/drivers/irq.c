#include "drivers/irq.h"
#include "common/list.h"
#include "common/utils.h"
#include "drivers/timer.h"

static irq_hadler_t irq_table[MAX_IRQ];

static list_head_t irq_task_free_list;
static irq_task_t irq_task_memory_pool[IRQ_TASK_MEMORY_POOL_SIZE];
static list_head_t irq_task_queue;

static uint32_t irq_level;
static uint32_t irq_current_priority;

void irq_init() {
  for (int i = 0; i < MAX_IRQ; i++) {
    irq_table[i] = NULL;
  }
  INIT_LIST_HEAD(&irq_task_queue);
  INIT_LIST_HEAD(&irq_task_free_list);
  for (int i = 0; i < IRQ_TASK_MEMORY_POOL_SIZE; i++) {
    INIT_LIST_HEAD(&irq_task_memory_pool[i].list);
    list_add_tail(&irq_task_memory_pool[i].list, &irq_task_free_list);
  }

  irq_current_priority = 0x3f3f3f3f;

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
  irq_level++;
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
  irq_level--;
  return;
}

void irq_task_enqueue(int priority, irq_task_handler_t handler) {
  irq_task_t *task =
      (irq_task_t *)list_entry(irq_task_free_list.next, irq_task_t, list);
  list_del(irq_task_free_list.next);
  // NOTE: Should handle allocation failure

  task->priority = priority;
  task->handler = handler;
  list_head_t *pos;
  list_for_each(pos, &irq_task_queue) {
    irq_task_t *t = container_of(pos, irq_task_t, list);
    if (t->priority < priority) {
      list_add_tail(&task->list, pos);
      return;
    }
  }
  list_add_tail(&task->list, &irq_task_queue);
}

irq_task_t *irq_task_dequeue() {
  if (list_empty(&irq_task_queue)) {
    return NULL;
  }
  list_head_t *pos = irq_task_queue.next;
  irq_task_t *task = container_of(pos, irq_task_t, list);
  list_del(pos);

  // NOTE: Should handle deallocation(free)

  return task;
}

irq_task_t *irq_task_peek() {
  if (list_empty(&irq_task_queue)) {
    return NULL;
  }
  list_head_t *pos = irq_task_queue.next;
  irq_task_t *task = container_of(pos, irq_task_t, list);
  return task;
}

void irq_task_exec() {
  irq_task_t *task = irq_task_peek();
  if (task == NULL) { // when no task in the queue
    return;
  }
  int irq_pre_priority = irq_current_priority;
  if (task->priority >= irq_pre_priority) {
    return;
  }
  irq_current_priority = task->priority;

  while (1) {
    task = irq_task_peek();
    if (task == NULL) { // when no task in the queue
      break;
    }
    if (task->priority >= irq_pre_priority) {
      break;
    }
    task = irq_task_dequeue();
    if (task == NULL) { // when no task in the queue
      break;
    }
    ENABLE_IRQ();
    task->handler();
    DISABLE_IRQ();
    list_add_tail(&task->list, &irq_task_free_list);
  }
  irq_current_priority = irq_pre_priority;
}
