#ifndef	_SYS_H
#define	_SYS_H

#include <stddef.h>

#define SYS_GETPID      0 		// syscall numbers 
#define SYS_UARTREAD    1 	
#define SYS_UARTWRITE   2 	
#define SYS_EXEC        3 	
#define SYS_FORK        4
#define SYS_EXIT        5
#define SYS_MBOX_CALL   6
#define KILL            7

int sys_getpid();
size_t sys_uartread(char buf[], size_t size);
size_t sys_uartwrite(const char buf[], size_t size);
int sys_exec(const char *name, char *const argv[]);
int sys_fork();
void sys_exit(int status);
int sys_mbox_call(unsigned char ch, unsigned int *mbox);
void sys_kill(int pid);

#endif  /*_SYS_H */