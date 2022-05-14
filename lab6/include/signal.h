#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <list.h>
#include <syscall.h>

typedef struct signal_info {
    int sig_num;
    void (*handler)();
    struct list_head list;
};

typedef struct signal_context {
    struct trap_frame* tf;
    void* user_stack;
};

extern void (*signal_table[])(int);

void signal_ignore(int pid);
void signal_kill(int pid);

void register_signal(int SIGNAL, void (*handler)());
void signal_call_sigreturn();

#endif  /* _SIGNAL_H */