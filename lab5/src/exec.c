#include <cpio.h>
#include <sched.h>
#include <allocator.h>

void exec_user_program()
{
    asm volatile (
        "mov x10, 0 \n\t"
        "msr spsr_el1, x10 \n\t"
        "msr elr_el1, %0 \n\t" 
        "msr sp_el0, %1 \n\t"
        "eret \n\t"
        ::  "r" (get_current()->data),
            "r" (get_current()->user_stack + PAGE_SIZE - sizeof(struct cpu_context))
    );
}

void exec_program(char* filename)
{
    struct file_info* fi = cpio_get_file(filename);

    struct task_struct* ts = new_task();
    ts->kernel_stack = kmalloc(PAGE_SIZE);
    ts->user_stack = kmalloc(PAGE_SIZE);

    ts->cpu_context.sp = ts->kernel_stack + PAGE_SIZE - sizeof(struct cpu_context);
    ts->cpu_context.pc = exec_user_program;
    ts->data = fi->data;
    ts->data_size = fi->data_size;

    sched_add_task(ts);
    return;
}