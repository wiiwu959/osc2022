#include <stddef.h>
#include <sched.h>
#include <allocator.h>
#include <string.h>
#include <list.h>

#include <printf.h>

struct list_head task_queue;
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
    ts->preempt_count = 0;
    ts->counter = 0;

    return ts;
}

void sched_init()
{
    top_id = 0;
    INIT_LIST_HEAD(&task_queue);
    struct task_struct *cur = new_task();

    set_current(cur);
    sched_add_task(cur);
}

void sched_add_task(struct task_struct* ts)
{
    printf("sched_add_task\n");
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
    if (ts->kernel_stack) {
        kfree(ts->kernel_stack);
    }
    kfree(ts);
    
    irq_restore(flags);
    preempt_enable();
}

void kthread_fin()
{
    sched_del_task(get_current());
    schedule();
}

void kthread_create(unsigned long fn, unsigned long arg)
{
    printf("kthread_create\n");
    struct task_struct* ts = new_task();

    ts->preempt_count = 1;
    ts->kernel_stack = kmalloc(PAGE_SIZE);

	ts->cpu_context.x19 = (unsigned long)fn;
	ts->cpu_context.x20 = arg;
	ts->cpu_context.pc = (unsigned long)ret_from_fork;
	ts->cpu_context.sp = ts->kernel_stack + PAGE_SIZE - sizeof(struct cpu_context);

    sched_add_task(ts);
}

static struct task_struct *next_task() {
    struct task_struct *ts = list_first_entry(&task_queue, struct task_struct, list);
	
    list_del(&ts->list);
	list_add_tail(&ts->list, &task_queue);

    return ts;
}

void schedule() 
{
    // printf("schedule\n");
    struct task_struct *cur = get_current();
    if (cur->preempt_count > 0) {
        return;
    }
    
    // printf("Current id = %d\r\n", get_current()->pid);
    size_t flags = disable_irq_save();
    struct task_struct *next = next_task();
    next->counter = DEFAULT_TIMEOUT;

    irq_restore(flags);
    switch_to(cur, next);
}

void timer_tick()
{
    get_current()->counter--;
    // printf("tick\n");

    if (get_current()->counter > 0 || get_current()->preempt_count > 0) {
        // printf("counter = %d\n", get_current()->counter);
        // printf("preempt_count = %d\n", get_current()->preempt_count);
        return;
    }

    enable_interrupt();
    schedule();
    disable_interrupt();
}