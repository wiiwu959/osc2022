#include <cpio.h>
#include <sched.h>
#include <allocator.h>
#include <printf.h>
#include <syscall.h>
#include <mmu.h>

void exec_user_program()
{
    asm volatile (
        "mov x10, 0 \n\t"
        "msr spsr_el1, x10 \n\t"
        "msr elr_el1, %0 \n\t" 
        "msr sp_el0, %1 \n\t"
        "eret \n\t"
        ::  "r" (0),
            "r" (0xffffffffb000 + STACK_SIZE)
    );
}

// exec user program from kernel
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

    map_pages(ts->page_table, 0, ts->data_size, virtual_to_physical(ts->data), PD_USER_RW | PD_USER_X);
    map_pages(ts->page_table, 0xffffffffb000, STACK_SIZE, virtual_to_physical(ts->user_stack), PD_USER_RW | PD_USER_NX);
    map_pages(ts->page_table, 0x3c000000, 0x04000000, 0x3c000000, PD_USER_RW | PD_USER_NX);

    sched_add_task(ts);
    return;
}

// TODO: exec parameter not handle yet
// exec use in user mode
// int exec_user(const char *name, char *const argv[])
int exec_user(struct trap_frame* regs)
{
    size_t flags = disable_irq_save();
    preempt_disable();

    char* name = regs->regs[0];
    char** arg = regs->regs[1];

    struct file_info* fi = cpio_get_file(name);

    current->pid = sched_newpid();
    
    current->data = fi->data;
    current->data_size = fi->data_size;
    kfree(fi);

    kfree(current->page_table);
    current->page_table = kmalloc(PAGE_SIZE);
    memset(current->page_table, 0, PAGE_SIZE);

    map_pages(current->page_table, 0, current->data_size, virtual_to_physical(current->data), PD_USER_RW | PD_USER_X);
    map_pages(current->page_table, 0xffffffffb000, STACK_SIZE, virtual_to_physical(current->user_stack), PD_USER_RW | PD_USER_NX);
    map_pages(current->page_table, 0x3c000000, 0x04000000, 0x3c000000, PD_USER_RW | PD_USER_NX);

    regs->pc = 0;
    regs->sp = 0xffffffffb000 + PAGE_SIZE * 4;

    // regs->regs[30] = ?? (set return)
    // regs->pc = fi->data;
    // regs->sp = current->user_stack + STACK_SIZE;

    irq_restore(flags);
    preempt_enable();

}