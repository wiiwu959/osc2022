#ifndef	_SYS_H
#define	_SYS_H

#include <stddef.h>

#define __NR_syscalls   12

#define SYS_GETPID      0 		// syscall numbers 
#define SYS_UARTREAD    1 	
#define SYS_UARTWRITE   2 	
#define SYS_EXEC        3 	
#define SYS_FORK        4
#define SYS_EXIT        5
#define SYS_MBOX_CALL   6
#define SYS_KILL        7
#define SYS_SIGNAL      8
#define SYS_SIGKILL     9
#define SYS_SIGRETURN   10
#define TEST            11

struct trap_frame {
    unsigned long regs[31];
    unsigned long sp;
    unsigned long pc;
    unsigned long pstate;
    // unsigned long sp_el0;
    // unsigned long elr_el1;
    // unsigned long spsr_el1;
};

void syscall_handler(struct trap_frame* regs);

// int sys_getpid();
void sys_getpid(struct trap_frame* regs);
// size_t sys_uartread(char buf[], size_t size);
void sys_uartread(struct trap_frame* regs);
// size_t sys_uartwrite(const char buf[], size_t size);
void sys_uartwrite(struct trap_frame* regs);
// int sys_exec(const char *name, char *const argv[]);
void sys_exec(struct trap_frame* regs);
// int sys_fork();
void sys_fork(struct trap_frame* regs);
// void sys_exit(int status);
void sys_exit(struct trap_frame* regs);
// int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_mbox_call(struct trap_frame* regs);
// void sys_kill(int pid);
void sys_kill(struct trap_frame* regs);
// void sys_test();
void sys_test(struct trap_frame* regs);

// signal
// void sys_signal(int SIGNAL, void (*handler)())
void sys_signal(struct trap_frame* regs);
// void sys_sigkill(int pid, int SIGNAL)
void sys_sigkill(struct trap_frame* regs);
void sys_sigreturn(struct trap_frame* regs);

#endif  /*_SYS_H */