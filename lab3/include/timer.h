#ifndef _TIMER_H
#define _TIMER_H

#define CORE0_TIMER_IRQ_CTRL ((unsigned int *)(0x40000040))

void core_timer_enable(void);
void core_timer_handler(void);

#endif  /* _TIMER_H */