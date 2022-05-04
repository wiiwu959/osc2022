#ifndef _SCHED_H
#define _SCHED_H

#include <stddef.h>
#include <list.h>
#include <syscall.h>
#include <signal.h>

#define TASK_RUNNING    0
#define TASK_DEAD       1

#define DEFAULT_TIMEOUT 15

#define STACK_SIZE 0x2000

#define current get_current()

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
    void* user_stack;
    void* data;
    int data_size;

    struct signal_context *sig_context;
    
    struct list_head sig_info;
    struct list_head list;
};

void preempt_enable();
void preempt_disable();

void schedule();
void sched_init();
int sched_newpid();
struct task_struct* sched_new_task();
void sched_add_task(struct task_struct* ts);
void sched_del_task(struct task_struct* ts);
void sched_kill_task(int id);
struct task_struct *sched_get_task(int pid);
struct task_struct *sched_next_task();
void sched_timer_tick();

// kthread
void kthread_create(unsigned long fn, unsigned long arg);
void kthread_fin();
void kthread_kill_zombies();
int kthread_fork(struct trap_frame* regs);

// asm
void switch_to(struct task_struct* prev, struct task_struct* next);
void set_current(struct task_struct *ts);
struct task_struct* get_current();
void kthread_func_wrapper(void);
void restore_regs_eret(void);
void call_sigreturn();



#endif  /* _SCHED_H */