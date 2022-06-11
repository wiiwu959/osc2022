#include <stddef.h>
#include <sched.h>
#include <allocator.h>
#include <list.h>
#include <mm.h>
#include <printf.h>
#include <syscall.h>
#include <mmu.h>
#include <fs/vfs.h>

struct list_head task_queue;
struct list_head dead_queue;

int top_id;

void preempt_disable()
{
	current->preempt_count++;
}

void preempt_enable()
{
	current->preempt_count--;
}

int sched_newpid()
{
    int cur = top_id;
    top_id++;
    return cur;
}

void schedule() 
{
    struct task_struct *cur = current;
    if (cur->preempt_count > 0) {
        return;
    }
    
    size_t flags = disable_irq_save();
    struct task_struct *next = sched_next_task();
    next->counter = DEFAULT_TIMEOUT;

    irq_restore(flags);
    switch_to(cur, next);
}

void sched_init()
{
    top_id = 0;
    INIT_LIST_HEAD(&task_queue);
    INIT_LIST_HEAD(&dead_queue);

    struct task_struct *cur = sched_new_task();

    set_current(cur);
    sched_add_task(cur);
}


struct task_struct* sched_new_task()
{
    struct task_struct *ts;
    ts = kmalloc(sizeof(struct task_struct));
    memset(ts, 0, sizeof(struct task_struct));

    ts->page_table = kmalloc(PAGE_SIZE);
    memset(ts->page_table, 0, PAGE_SIZE);
    
    ts->pid = top_id++;
    ts->kernel_stack = NULL;
    ts->user_stack = NULL;
    ts->data = NULL;
    ts->data_size = 0;
    ts->preempt_count = 0;
    ts->counter = 0;

    ts->cwd = rootfs->root;
    ts->fd_num = -1;
    for (int i = 0; i < 16; i++) {
        ts->fd_table[i] = NULL;
    }

    INIT_LIST_HEAD(&ts->sig_info);
    ts->sig_context = NULL;

    return ts;
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
    list_add_tail(&ts->list, &dead_queue);

    for (int i = 0; i <= ts->fd_num; i++) {
        if (ts->fd_table[i]->vnode != NULL) {
            vfs_close(ts->fd_table[i]);
        }
    }

    irq_restore(flags);
    preempt_enable();
}

void sched_kill_task(int id)
{
    struct task_struct* ts;
    list_for_each_entry(ts, &task_queue, list) {
        if (ts->pid == id) {
            sched_del_task(ts);
            printf("[*] Pid %d killed.\r\n", id);
            return;
        }
    }
    printf("[*] Pid %d is not running.\r\n", id);
    return;
}

struct task_struct *sched_get_task(int pid)
{
    struct task_struct* ts;
    list_for_each_entry(ts, &task_queue, list) {
        if (ts->pid == pid) {
            return ts;
        }
    }
    return NULL;
}

struct task_struct *sched_next_task() {
    struct task_struct *ts = list_first_entry(&task_queue, struct task_struct, list);

    list_del(&ts->list);
    list_add_tail(&ts->list, &task_queue);

    return ts;
}

void sched_timer_tick()
{
    current->counter--;

    if (current->counter > 0 || current->preempt_count > 0) {
        return;
    }

    enable_interrupt();
    schedule();
    disable_interrupt();
}

void kthread_create(unsigned long fn, unsigned long arg)
{
    struct task_struct* ts = sched_new_task();

    ts->preempt_count = 1;
    ts->kernel_stack = kmalloc(STACK_SIZE);

	ts->cpu_context.x19 = (unsigned long)fn;
	ts->cpu_context.x20 = arg;
	ts->cpu_context.pc = (unsigned long)kthread_func_wrapper;
    // need to align 16 or else it will die
	ts->cpu_context.sp = ts->kernel_stack + STACK_SIZE - sizeof(struct cpu_context) - 12;

    sched_add_task(ts);
}

void kthread_fin()
{
    sched_del_task(current);
    schedule();
}

void kthread_kill_zombies()
{
    while (!list_empty(&dead_queue)) {
        struct task_struct *dead = list_first_entry(&dead_queue, struct task_struct, list);
        list_del(&dead->list);
        // if (dead->kernel_stack) {
        //     kfree(dead->kernel_stack);
        //     kfree(dead->user_stack);
        // }
        kfree(dead);
    }
}


int kthread_fork(struct trap_frame* regs) 
{
    struct task_struct* child = sched_new_task();
    // struct task_struct* cur = current;

    child->preempt_count = 0;
    child->state = current->state;
    child->counter = 0;

    child->kernel_stack = kmalloc(STACK_SIZE);
    child->user_stack = kmalloc(STACK_SIZE);
    memcpy(child->user_stack, current->user_stack, STACK_SIZE);
    memcpy(child->kernel_stack, current->kernel_stack, STACK_SIZE);

    // child->data = current->data;
    child->data = kmalloc(current->data_size);
    memcpy(child->data, current->data, current->data_size);
    child->data_size = current->data_size;

    memcpy(&child->cpu_context, &current->cpu_context, sizeof(struct cpu_context));
    
    struct trap_frame* child_frame = (struct trap_frame*)((size_t)child->kernel_stack + STACK_SIZE - sizeof(struct trap_frame));
    memcpy(child_frame, regs, sizeof(struct trap_frame));
    
    child_frame->regs[0] = 0;
    
    child->cpu_context.sp = child_frame;
    child->cpu_context.pc = restore_regs_eret;

    map_pages(child->page_table, 0, child->data_size, virtual_to_physical(child->data), PD_USER_RW | PD_USER_X);
    map_pages(child->page_table, 0xffffffffb000, STACK_SIZE, virtual_to_physical(child->user_stack), PD_USER_RW | PD_USER_NX);
    map_pages(child->page_table, 0x3c000000, 0x04000000, 0x3c000000, PD_USER_RW | PD_USER_NX);

    size_t flags = disable_irq_save();
    sched_add_task(child);
    irq_restore(flags);
    return child->pid;
}