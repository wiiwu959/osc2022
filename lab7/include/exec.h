#ifndef _EXEC_H
#define _EXEC_H

// exececute a program from cpio in user space
void exec_program(char* filename);

// from el1 to el0
void exec_user_program();

// exec for syscall exec: replace current task and pid++
int exec_user(struct trap_frame* regs);

#endif   /* _EXEC_H */