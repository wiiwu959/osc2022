#include <mini_uart.h>
#include <timer.h>
#include <shell.h>

void main(char* fdt)
{
    uart_init();
    initramfs_init(fdt);

    enable_interrupt();
    timer_init();
    
    uart_send_string("Hello! Type command to start.\r\n");
    shell();
}