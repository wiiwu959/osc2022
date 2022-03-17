#include <mini_uart.h>
#include <reboot.h>
#include <mailbox.h>
#include <string.h>

// extern unsigned int _kernel[];
// extern char _start;
char buf[0x200];
char *fdt_base;

void bootloader_main(char* fdt)
{
    uart_init();
    uart_send_string("Hello! Start loading kernel.\r\n");

    int kernel_size;
    char* kernel = (char *)0x80000;

    // about to delete
    uart_send_string("bootloader_main : ");
    uart_send_hex((unsigned int)bootloader_main);
    uart_send_string("\r\n");

    uart_send_string("Kernel size : ");
    uart_recv();
    uart_recvn((char*)&kernel_size, 4);
    uart_send_hex(kernel_size);
    uart_send_string("\r\n");
    uart_send_string("Kernel loaded at 0x80000.\r\n");

    uart_recvn(kernel, kernel_size);

    ((void (*)())kernel)(fdt);
}

