#ifndef __IRQ_H
#define __IRQ_H

#include "common/list.h"
#include "common/types.h"
#include "common/utils.h"

#define MAX_IRQ 64
#define IRQ_BASE_REG (MMIO_BASE + 0xB000)
#define IRQ_PENDINGB_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x200))
#define IRQ_PENDING1_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x204))
#define IRQ_PENDING2_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x208))
#define IRQ_ENABLE1_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x210))
#define IRQ_ENABLE2_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x214))
#define IRQ_ENABLEB_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x218))
#define IRQ_DISABLE1_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x21C))
#define IRQ_DISABLE2_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x220))
#define IRQ_DISABLEB_REG ((volatile uint32_t *)(IRQ_BASE_REG + 0x224))

typedef void (*irq_hadler_t)(void);

void irq_init();

void irq_register(int irq_num, irq_hadler_t handler);

void irq_enable(int irq_num);

void irq_disable(int irq_num);

void irq_handler_entry();

#define ENABLE_IRQ() asm volatile("msr daifclr, #0xf" ::: "memory")

#define DISABLE_IRQ() asm volatile("msr daifset, #0xf" ::: "memory")

#define IRQ_TASK_MEMORY_POOL_SIZE 100

typedef void (*irq_task_handler_t)(void);

typedef struct irq_task {
  int priority;
  irq_task_handler_t handler;
  list_head_t list;
} irq_task_t;

void irq_task_enqueue(int proirity, irq_task_handler_t handler);

irq_task_t *irq_task_dequeue();

irq_task_t *irq_task_peek();

void irq_task_exec();

#endif // __IRQ_H
