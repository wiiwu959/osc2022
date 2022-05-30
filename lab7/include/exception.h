#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <stddef.h>

int get_el(void);
void from_el2_to_el1(void);
void enable_interrupt(void);
void disable_interrupt(void);

size_t disable_irq_save();
void irq_restore(size_t flag);

#endif  /* _EXCEPTION_H */