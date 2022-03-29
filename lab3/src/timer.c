#include <timer.h>
#include <printf.h>

void core_timer_enable()
{
    asm volatile (
        "mov x0, 1 \n\t"
        "msr cntp_ctl_el0, x0 \n\t"     // enable
        "mrs x0, cntfrq_el0 \n\t"
        "msr cntp_tval_el0, x0 \n\t"    // set expired time
    );
    
    *CORE0_TIMER_IRQ_CTRL = 2;
}

void core_timer_handler()
{
    unsigned long pct, frq;
    asm volatile("mrs %0, cntpct_el0" : "=r" (pct));
    asm volatile("mrs %0, cntfrq_el0" : "=r" (frq));

    unsigned long seconds = pct / frq;
    printf("Seconds after booting: %d\n", seconds);
    asm volatile("msr cntp_tval_el0, %0" :: "r" (frq * 2));
    return;
}


