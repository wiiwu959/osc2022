#include <syscall.h>
#include <stddef.h>
#include <mini_uart.h>
#include <sched.h>
#include <mailbox.h>
#include <printf.h>
#include <signal.h>
#include <allocator.h>
#include <mm.h>

void sys_getpid(struct trap_frame* regs)
{
    regs->regs[0] = current->pid;
}

void sys_uartread(struct trap_frame* regs)
{
    char* buf = regs->regs[0];
    size_t size = regs->regs[1];
    uart_recvn(buf, size);
    regs->regs[0] = size;
}

void sys_uartwrite(struct trap_frame* regs)
{
    char* buf = regs->regs[0];
    size_t size = regs->regs[1];
    uart_sendn(buf, size);
    regs->regs[0] = size;
}

void sys_exec(struct trap_frame* regs)
{
    exec_user(regs);
    regs->regs[0] = 0;
}

void sys_fork(struct trap_frame* regs)
{
    int pid = kthread_fork(regs);
    regs->regs[0] = pid;
}

void sys_exit(struct trap_frame* regs)
{
    kthread_fin();
}

void sys_mbox_call(struct trap_frame* regs)
{
    unsigned char ch = regs->regs[0];
    unsigned int* mbox;
    mbox = regs->regs[1];
    regs->regs[0] = mailbox_call(ch, mbox);
}

void sys_kill(struct trap_frame* regs)
{
    int pid = regs->regs[0];
    sched_kill_task(pid);
}


void sys_signal(struct trap_frame* regs)
{
    int SIGNAL = regs->regs[0];
    void (*handler) = regs->regs[1];

    preempt_disable();
    register_signal(SIGNAL, handler);
    preempt_enable();
}

// void sys_sigkill(int pid, int SIGNAL)
void sys_sigkill(struct trap_frame* regs)
{
    int pid = regs->regs[0];
    int SIGNAL = regs->regs[1];

    struct task_struct* ts = sched_get_task(pid);

    struct signal_info* target;
    list_for_each_entry(target, &current->sig_info, list) {
        if (target->sig_num == SIGNAL) {
            current->sig_context = kmalloc(sizeof(struct signal_context));
            current->sig_context->tf = kmalloc(sizeof(struct trap_frame));
            memcpy(current->sig_context->tf, regs, sizeof(struct trap_frame));
            current->sig_context->user_stack = kmalloc(STACK_SIZE);

            regs->regs[30] = signal_call_sigreturn;
            regs->pc = target->handler;
            regs->sp = current->sig_context->user_stack + STACK_SIZE;

            return;
        }
    }

    // If signal not registered, use default handler
    // sched_kill_task(pid);
    signal_table[SIGNAL](pid);
    return;
}

void sys_sigreturn(struct trap_frame* regs)
{
    preempt_disable();

    memcpy((char *)regs, (char *)current->sig_context->tf, sizeof(struct trap_frame));
    kfree(current->sig_context->tf);
    kfree(current->sig_context->user_stack);
    kfree(current->sig_context);
    current->sig_context = NULL;

    preempt_enable();
}

void sys_test(struct trap_frame* regs)
{
    printf("[*] Testing syscall 8\r\n");
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
    (func) sys_signal,
    (func) sys_sigkill,
    (func) sys_sigreturn,
    (func) sys_test
};

void syscall_handler(struct trap_frame* regs)
{
    unsigned nr = regs->regs[8]; // syscall number
    if (nr >= __NR_syscalls) {
        printf("Invalid syscall.\r\n");
        return;
    }
    (syscall_table[nr])(regs);
}
