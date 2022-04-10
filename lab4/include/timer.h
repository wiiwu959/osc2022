#ifndef _TIMER_H
#define _TIMER_H

#define CORE0_TIMER_IRQ_CTRL 0x40000040

// time_event
typedef struct _time_event
{
    int start_time;
    int duration;
    int expired_time;

    struct _time_event *prev;
    struct _time_event *next;

    char msg[0x100];
    void (*callback)(char*);
} time_event;

void timer_init();

void core_timer_enable(void);
void core_timer_disable(void);
void core_timer_handler(void);

void each_timer_handler(void);

void set_timeout(int sec);
unsigned long get_current_time();

void add_timer(void (*callback)(char*), int sec, char* msg);

#endif  /* _TIMER_H */