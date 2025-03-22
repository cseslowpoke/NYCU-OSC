#ifndef __TIMER_H
#define __TIMER_H

#define CORE0_TIMER_IRQ_CTRL ((volatile uint32_t *)0x40000040)

void timer_init(void);

void timer_enable(void);

void timer_irq_handler(void);

#endif // __TIMER_H
