#include <mini_uart.h>
#include <timer.h>
#include <shell.h>
#include <exception.h>
#include <sched.h>


void idle()
{
    while (1) {
        schedule();
        kthread_kill_zombies();
    }
}

void main(char* fdt)
{
    uart_init();
    mm_init(fdt);
    timer_init();
    sched_init();
    
    kthread_create(&shell, NULL);
    enable_interrupt();
    idle();
}