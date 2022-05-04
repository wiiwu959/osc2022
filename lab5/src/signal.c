#include <list.h>
#include <signal.h>
#include <allocator.h>
#include <sched.h>

typedef void (*func)(int);
func signal_table[] = {
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_ignore,
    (func) signal_kill
};

void signal_ignore(int pid)
{
    return;
}

void signal_kill(int pid)
{
    sched_kill_task(pid);
}

void register_signal(int SIGNAL, void (*handler)())
{
    struct signal_info* new = kmalloc(sizeof(struct signal_info));
    new->sig_num = SIGNAL;
    new->handler = handler;

    list_add_tail(&new->list, &current->sig_info);
}

void signal_call_sigreturn()
{
    asm volatile(
        "mov x8, 0xa\r\n"
        "svc 0\r\n"
    );
}