#include <cpio.h>
#include <sched.h>
#include <allocator.h>
#include <printf.h>
#include <syscall.h>

void exec_user_program()
{
    asm volatile (
        "mov x10, 0 \n\t"
        "msr spsr_el1, x10 \n\t"
        "msr elr_el1, %0 \n\t" 
        "msr sp_el0, %1 \n\t"
        "eret \n\t"
        ::  "r" (current->data),
            "r" (current->user_stack + STACK_SIZE)
    );
}

void exec_program(char* filename)
{
    struct file_info* fi = cpio_get_file(filename);

    if (fi == NULL) {
        printf("[*] exec program failed.\r\n");
        return;
    }

    struct task_struct* ts = sched_new_task();
    ts->kernel_stack = kmalloc(STACK_SIZE);
    ts->user_stack = kmalloc(STACK_SIZE);

    ts->cpu_context.sp = ts->kernel_stack + STACK_SIZE;
    ts->cpu_context.pc = exec_user_program;
    ts->data = fi->data;
    ts->data_size = fi->data_size;

    kfree(fi);

    sched_add_task(ts);
    return;
}

// int exec_user(const char *name, char *const argv[])
int exec_user(struct trap_frame* regs)
{

    size_t flags = disable_irq_save();
    preempt_disable();

    char* name = regs->regs[0];
    char** arg = regs->regs[1];

    struct file_info* fi = cpio_get_file(name);
    struct task_struct *ts = current;

    current->pid = sched_newpid();

    // regs->regs[30] = fi->data;
    // regs->regs[30] = current->data;
    regs->pc = fi->data;
    regs->sp = current->user_stack + STACK_SIZE;

    kfree(fi);

    irq_restore(flags);
    preempt_enable();

}