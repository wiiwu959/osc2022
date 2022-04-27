#include <stddef.h>
#include <sched.h>
#include <allocator.h>
#include <list.h>
#include <mm.h>
#include <printf.h>
#include <syscall.h>

struct list_head task_queue;
struct list_head dead_queue;

int top_id;

void preempt_disable()
{
	get_current()->preempt_count++;
}

void preempt_enable()
{
	get_current()->preempt_count--;
}

struct task_struct* new_task()
{
    struct task_struct *ts;
    ts = kmalloc(sizeof(struct task_struct));
    memset(ts, 0, sizeof(struct task_struct));

    ts->pid = top_id++;
    ts->kernel_stack = NULL;
    ts->user_stack = NULL;
    ts->data = NULL;
    ts->data_size = 0;
    ts->preempt_count = 0;
    ts->counter = 0;

    return ts;
}

void sched_init()
{
    top_id = 0;
    INIT_LIST_HEAD(&task_queue);
    INIT_LIST_HEAD(&dead_queue);

    struct task_struct *cur = new_task();

    set_current(cur);
    sched_add_task(cur);
}

void sched_add_task(struct task_struct* ts)
{
    preempt_disable();
    size_t flags = disable_irq_save();

    list_add_tail(&ts->list, &task_queue);
    
    irq_restore(flags);
    preempt_enable();
}

void sched_del_task(struct task_struct* ts)
{
    preempt_disable();
    size_t flags = disable_irq_save();

    list_del(&ts->list);
    ts->state = TASK_DEAD;
    list_add_tail(ts, &dead_queue);
    
    irq_restore(flags);
    preempt_enable();
}

void sched_kill_task(int id)
{
    preempt_disable();
    size_t flags = disable_irq_save();

    struct task_struct* ts;
    list_for_each_entry(ts, &task_queue, list) {
        if (ts->pid == id) {
            list_del(&ts->list);
            ts->state = TASK_DEAD;
            list_add_tail(ts, &dead_queue);
            printf("Pid %d killed.\r\n", id);
            return;
        }
    }
    printf("[*] Pid %d is not running.\r\n", id);
    return;

    
    irq_restore(flags);
    preempt_enable();
}

void kthread_create(unsigned long fn, unsigned long arg)
{
    struct task_struct* ts = new_task();

    ts->preempt_count = 1;
    ts->kernel_stack = kmalloc(PAGE_SIZE);

	ts->cpu_context.x19 = (unsigned long)fn;
	ts->cpu_context.x20 = arg;
	ts->cpu_context.pc = (unsigned long)ret_from_fork;
	ts->cpu_context.sp = ts->kernel_stack + PAGE_SIZE - sizeof(struct cpu_context);

    sched_add_task(ts);
}

void kthread_fin()
{
    sched_del_task(get_current());
    schedule();
}

void kthread_kill_zombies()
{
    while (!list_empty(&dead_queue)) {
        struct task_struct *dead = list_first_entry(&dead_queue, struct task_struct, list);
        if (dead->kernel_stack) {
            kfree(dead->kernel_stack);
            kfree(dead->user_stack);
        }
        kfree(dead);
    }
}

static struct task_struct *next_task() {
    struct task_struct *ts = list_first_entry(&task_queue, struct task_struct, list);

    list_del(&ts->list);
    list_add_tail(&ts->list, &task_queue);

    return ts;
}

void schedule() 
{
    struct task_struct *cur = get_current();
    if (cur->preempt_count > 0) {
        return;
    }
    
    size_t flags = disable_irq_save();
    struct task_struct *next = next_task();
    next->counter = DEFAULT_TIMEOUT;

    irq_restore(flags);
    switch_to(cur, next);
}

void sched_timer_tick()
{
    get_current()->counter--;

    if (get_current()->counter > 0 || get_current()->preempt_count > 0) {
        return;
    }

    enable_interrupt();
    schedule();
    disable_interrupt();
}


int kthread_fork(struct trap_frame* regs) 
{  
 

}