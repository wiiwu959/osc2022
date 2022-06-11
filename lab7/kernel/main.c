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
#include <fs/tmpfs.h>
#include <fs/cpiofs.h>

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

    kernel_space_mapping();

    // filesystem init
    vfs_init();
    register_filesystem(&tmpfs);
    vfs_mount_rootfs("tmpfs");

    register_filesystem(&cpiofs);
    vfs_mkdir("/initramfs");
    vfs_mount("/initramfs", "cpiofs");

    exec_program("/initramfs/vfs1.img");

    enable_interrupt();
    // shell();
    // kthread_create(&shell, NULL);
    idle();
}