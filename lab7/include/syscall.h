#ifndef	_SYS_H
#define	_SYS_H

#include <stddef.h>

#define __NR_syscalls   18

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
#define SYS_OPEN        11
#define SYS_CLOSE       12
#define SYS_WRITE       13
#define SYS_READ        14
#define SYS_MKDIR       15
#define SYS_MOUNT       16
#define SYS_CHDIR       17

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

// signal
// void sys_signal(int SIGNAL, void (*handler)())
void sys_signal(struct trap_frame* regs);
// void sys_sigkill(int pid, int SIGNAL)
void sys_sigkill(struct trap_frame* regs);
void sys_sigreturn(struct trap_frame* regs);

// file system
// int open(const char *pathname, int flags);
void sys_open(struct trap_frame* regs);
// int close(int fd);
void sys_close(struct trap_frame* regs);
// long write(int fd, const void *buf, unsigned long count);
void sys_write(struct trap_frame* regs);
// long read(int fd, void *buf, unsigned long count);
void sys_read(struct trap_frame* regs);
// int mkdir(const char *pathname, unsigned mode);
void sys_mkdir(struct trap_frame* regs);
// int mount(const char *src, const char *target, const char *filesystem, unsigned long flags, const void *data);
void sys_mount(struct trap_frame* regs);
// int chdir(const char *path);
void sys_chdir(struct trap_frame* regs);

#endif  /*_SYS_H */