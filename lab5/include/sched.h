#ifndef _SCHED_H
#define _SCHED_H

#include <stddef.h>
#include <list.h>

#define TASK_RUNNING    0

#define DEFAULT_TIMEOUT 15

struct cpu_context {
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long sp;
    unsigned long pc;
};

struct task_struct {
    struct cpu_context cpu_context;
    int pid;
    long state;
    long counter;
    long preempt_count;

    void* kernel_stack;

    struct list_head list;
};

void preempt_enable();
void preempt_disable();

struct task_struct* new_task();

void sched_init();
void schedule();
void sched_add_task(struct task_struct* ts);
void sched_del_task(struct task_struct* ts);

// kthread
void kthread_create(unsigned long fn, unsigned long arg);
void kthread_fin();

void timer_tick();

// asm
void switch_to(struct task_struct* prev, struct task_struct* next);
void set_current(struct task_struct *ts);
struct task_struct* get_current();
void ret_from_fork(void);


#endif  /* _SCHED_H */