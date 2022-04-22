#include <mini_uart.h>
#include <timer.h>
#include <shell.h>
#include <exception.h>
#include <allocator.h>

void main(char* fdt)
{
    uart_init();

    enable_interrupt();
    timer_init();

    mm_init(fdt);
    
    uart_send_string("Hello! Type command to start.\r\n");
    shell();
}