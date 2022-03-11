#include <mini_uart.h>
#include <reboot.h>
#include <mailbox.h>
#include <string.h>

char buf[0x200];
void main(void)
{
    uart_init();
    uart_sendline("Hello! Type command to start.\r\n");

    while (1) {
        uart_sendline("# ");
        uart_recvline(buf);
        uart_sendline("\r\n");

        if (strcmp("help", buf) == 0) {
            uart_sendline("help     :print this help menu\r\n");
            uart_sendline("hello    :print Hello World!\r\n");
            uart_sendline("reboot   :reboot the device\r\n");
            uart_sendline("hardware :get hardware's info\r\n");
        } else if (strcmp("hello", buf) == 0) {
            uart_sendline("Hello World!\r\n");
        } else if (strcmp("reboot", buf) == 0) {
            uart_sendline("Rebooting...\r\n");
            uart_reset(5);
        } else if (strcmp("hardware", buf) == 0) {
            uart_sendline("board revision: ");
            unsigned int revision = get_board_revision();
            uart_send_hex(revision);
            uart_sendline("\r\n");

            struct arm_memory mem_info = get_arm_memory();
            uart_sendline("ARM memory:\r\n");
            uart_sendline("  base address: ");
            uart_send_hex(mem_info.base_addr);
            uart_sendline("\r\n");
            uart_sendline("  size        : ");
            uart_send_hex(mem_info.size);
            uart_sendline("\r\n");
        } else {
            uart_sendline("Command not found\r\n");
            uart_sendline("Use \"help\" to see help \r\n");
        }
    }
}