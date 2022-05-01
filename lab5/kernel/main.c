#include <mini_uart.h>
#include <timer.h>
#include <shell.h>
#include <exception.h>
#include <sched.h>
#include <printf.h>
#include <exec.h>
#include <allocator.h>


void idle()
{
    while (1) {
        schedule();
        kthread_kill_zombies();
    }
}

void foo()
{
    for(int i = 0; i < 10; ++i) {
        printf("Thread id: %d %d\n", current->pid, i);
        delay(1000000);
        schedule();
    }
}

void main(char* fdt)
{
    uart_init();
    mm_init(fdt);
    timer_init();
    sched_init();

    for (int i = 0; i < 5; i++) {
        kthread_create(&foo, "foo");    
    }

    printf("");
    exec_program("syscall.img");
    
    enable_interrupt();
    // kthread_create(&shell, NULL);
    idle();
}