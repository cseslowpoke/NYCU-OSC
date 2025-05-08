#ifndef __WATCH_DOG_H
#define __WATCH_DOG_H

#include "common/utils.h"
#define PM_PASSWORD 0x5a000000
#define PM_RSTC (MMIO_BASE + 0x10001c)
#define PM_WDOG (MMIO_BASE + 0x100024)

void set(long addr, unsigned int value);
void reset(int tick); // reboot after watchdog timer expire
#endif
