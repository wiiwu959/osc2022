#ifndef _REBOOT_H
#define _REBOOT_H

#include <mmu.h>

#define PM_PASSWORD (PHYS_OFFSET + 0x5a000000)
#define PM_RSTC (PHYS_OFFSET + 0x3F10001c)
#define PM_WDOG (PHYS_OFFSET + 0x3F100024)

void set(long addr, unsigned int value);
void uart_reset(int tick);
void uart_cancel_reset();

#endif  /*_REBOOT_H */