#ifndef _REBOOT_H
#define _REBOOT_H

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value);
void uart_reset(int tick);
void uart_cancel_reset();

#endif  /*_REBOOT_H */