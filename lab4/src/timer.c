#include <timer.h>
#include <printf.h>
#include <stddef.h>
#include <allocator.h>
#include <util.h>

time_event *time_event_head;

void core_timer_enable()
{
    asm volatile (
        "mov x0, 1 \n\t"
        "msr cntp_ctl_el0, x0 \n\t"     // enable
    );
    // set expired time
    unsigned long frq;
    asm volatile("mrs %0, cntfrq_el0" : "=r" (frq));
    asm volatile("msr cntp_tval_el0, %0" :: "r" (frq * 2));
    put(CORE0_TIMER_IRQ_CTRL, 2);
}

void core_timer_disable()
{
    put(CORE0_TIMER_IRQ_CTRL, 2);
}



void core_timer_handler()
{
    unsigned long pct, frq;
    asm volatile("mrs %0, cntpct_el0" : "=r" (pct));
    asm volatile("mrs %0, cntfrq_el0" : "=r" (frq));

    unsigned long seconds = pct / frq;
    printf("Seconds after booting: %d\r\n", seconds);
    asm volatile("msr cntp_tval_el0, %0" :: "r" (frq * 2));
    return;
}

void each_timer_handler()
{
    unsigned long pct, frq;
    asm volatile("mrs %0, cntpct_el0" : "=r" (pct));
    asm volatile("mrs %0, cntfrq_el0" : "=r" (frq));

    unsigned long seconds = pct / frq;
    
    asm volatile("msr cntp_tval_el0, %0" :: "r" (frq));
    
    if (time_event_head) {
        if (time_event_head->expired_time == seconds) {
            printf("\r\n[*] start time: %d, duration: %d, expired time: %d\r\n", \
                    time_event_head->start_time, time_event_head->duration, time_event_head->expired_time);
            time_event_head->callback(time_event_head->msg);
            time_event_head = time_event_head->next;
            kfree(time_event_head->prev);
            time_event_head->prev = NULL;
            
        }
    }
    return;
}

unsigned long get_current_time()
{
    unsigned long pct, frq;
    asm volatile("mrs %0, cntpct_el0" : "=r" (pct));
    asm volatile("mrs %0, cntfrq_el0" : "=r" (frq));

    unsigned long seconds = pct / frq;
    return seconds;
}

void timer_init() 
{
    core_timer_enable();
    time_event_head = NULL;
}

void add_timer(void (*callback)(char*), int sec, char* msg)
{
    time_event* add_event = kmalloc(sizeof(time_event));
    add_event->start_time = get_current_time();
    add_event->duration = sec;
    add_event->expired_time = add_event->start_time + sec;
    int i = 0;
    while (*msg != '\0') {
        add_event->msg[i] = *msg;
        i++;
        msg++;
    }
    add_event->msg[i] = '\0';

    add_event->callback = callback;
    
    if (time_event_head == NULL) {
        add_event->next = NULL;
        add_event->prev = NULL;
        time_event_head = add_event;
    } else {
        time_event* cur = time_event_head;

        // insert to queue head
        if (cur->expired_time > add_event->expired_time) {
            add_event->next = cur;
            add_event->prev = NULL;
            cur->prev = add_event;
            time_event_head = add_event;
        } else {
            while (1) {
                // insert between cur and cur->next
                if (cur->next) {
                    if (cur->next->expired_time > add_event->expired_time) {
                        time_event* tmp = cur->next;
                        cur->next = add_event;
                        tmp->prev = add_event;
                        add_event->next = tmp;
                        add_event->prev = cur;
                        break;
                    }
                    cur = cur->next;
                // insert to queue tail
                } else {
                    add_event->next = NULL;
                    add_event->prev = cur;
                    cur->next = add_event;
                    break;
                }
            }
        }
    }
    
}

