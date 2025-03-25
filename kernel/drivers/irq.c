#include "drivers/irq.h"
#include "common/utils.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "fs/fdt.h"

irq_hadler_t irq_table[MAX_IRQ];

void irq_init() {
  for (int i = 0; i < MAX_IRQ; i++) {
    irq_table[i] = NULL;
  }
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
  DISABLE_IRQ();
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
  ENABLE_IRQ();
  return;
}
