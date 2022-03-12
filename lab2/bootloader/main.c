#include <mini_uart.h>
#include <reboot.h>
#include <mailbox.h>
#include <string.h>

// extern unsigned int _kernel[];
// extern char _start;
char buf[0x200];

void bootloader_main(void)
{
    uart_init();
    uart_sendline("Hello! Start loading kernel.\r\n");
    uart_recvline(buf);

    int kernel_size;
    char* kernel = (char *)0x80000;

    uart_sendline("bootloader_main : ");
    uart_send_hex((unsigned int)bootloader_main);
    uart_sendline("\r\n");

    uart_sendline("Kernel size : ");
    uart_recvn((char*)&kernel_size, 4);
    uart_send_hex(kernel_size);
    uart_sendline("\r\n");

    for (int i = 0; i < kernel_size; i++) {
        *kernel = uart_recv();
        kernel++;
    }

    uart_sendline("\r\n");
    uart_sendline("Kernel loaded at 0x80000.\r\n");
}

