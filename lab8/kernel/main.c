#include <mini_uart.h>
#include <timer.h>
#include <shell.h>
#include <exception.h>
#include <sched.h>
#include <printf.h>
#include <exec.h>
#include <allocator.h>
#include <mmu.h>
#include <fs/vfs.h>

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

    kernel_space_mapping();

    vfs_init();    
    sched_init();

    exec_program("/initramfs/vfs1.img");

    enable_interrupt();
    // shell();
    // kthread_create(&shell, NULL);
    idle();
}