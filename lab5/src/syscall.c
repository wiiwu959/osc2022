#include <syscall.h>
#include <stddef.h>
#include <mini_uart.h>
#include <sched.h>
#include <mailbox.h>
#include <printf.h>

void sys_getpid(struct trap_frame* regs)
{
    printf("sys_getpid\n");
    regs->regs[0] = current->pid;
}

void sys_uartread(struct trap_frame* regs)
{
    char* buf = regs->regs[0];
    size_t size = regs->regs[1];
    // size_t flags = disable_irq_save();
    uart_recvn(buf, size);
    // irq_restore(flags);
    regs->regs[0] = size;
}

void sys_uartwrite(struct trap_frame* regs)
{
    char* buf = regs->regs[0];
    size_t size = regs->regs[1];
    uart_sendn(buf, size);
    regs->regs[0] = size;
}

// Fuck ?? Maybe not that bad?
void sys_exec(struct trap_frame* regs)
{
    printf("sys_exec\n");

    char* name = regs->regs[0];
    char** arg = regs->regs[1];
    exec_user(regs);
    regs->regs[0] = 0;
}

// Fuck!
void sys_fork(struct trap_frame* regs)
{
    printf("sys_fork\n");

    int pid = kthread_fork(regs);
    regs->regs[0] = pid;
}

void sys_exit(struct trap_frame* regs)
{
    printf("sys_exit\n");

    kthread_fin();
}

void sys_mbox_call(struct trap_frame* regs)
{
    printf("sys_mbox_call\n");

    unsigned char ch = (unsigned char)regs->regs[0];
    unsigned int* mbox = (unsigned int*)regs->regs[1];
    regs->regs[0] = mailbox_call(ch, mbox);
}

void sys_kill(struct trap_frame* regs)
{
    printf("sys_kill\n");

    int pid = regs->regs[0];
    sched_kill_task(pid);
}

void sys_test(struct trap_frame* regs)
{
    printf("[*] Testing syscall 8\r\n");
    // printf("elr_el1\t%x\r\n", regs->elr_el1);
    // printf("sp_el0\t%x\r\n", regs->sp_el0);
    // printf("spsr_el1\t%x\r\n", regs->spsr_el1);
}

typedef void *(*func)(struct trap_frame*);
func syscall_table[] = {
    (func) sys_getpid,
    (func) sys_uartread,
    (func) sys_uartwrite,
    (func) sys_exec,
    (func) sys_fork,
    (func) sys_exit,
    (func) sys_mbox_call,
    (func) sys_kill,
    (func) sys_test
};

void syscall_handler(struct trap_frame* regs)
{
    unsigned nr = regs->regs[8]; // syscall number
    if (nr >= __NR_syscalls) {
        printf("Invalid syscall.\r\n");
        return;
    }
    // printf("nr = %d\n",nr);
    (syscall_table[nr])(regs);
}
