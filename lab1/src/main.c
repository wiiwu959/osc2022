#include <mini_uart.h>
#include <reboot.h>
#include <mailbox.h>
#include <string.h>

char buf[0x200];
void main(void)
{
    uart_init();
    uart_send_string("Hello! Type command to start.\r\n");

    while (1) {
        uart_send_string("# ");
        uart_recv_line(buf);
        uart_send_string("\r\n");

        if (strcmp("help", buf) == 0) {
            uart_send_string("help     :print this help menu\r\n");
            uart_send_string("hello    :print Hello World!\r\n");
            uart_send_string("reboot   :reboot the device\r\n");
            uart_send_string("hardware :get hardware's info\r\n");
        } else if (strcmp("hello", buf) == 0) {
            uart_send_string("Hello World!\r\n");
        } else if (strcmp("reboot", buf) == 0) {
            uart_send_string("Rebooting...\r\n");
            uart_reset(5);
        } else if (strcmp("hardware", buf) == 0) {
            uart_send_string("board revision: ");
            unsigned int revision = get_board_revision();
            uart_send_hex(revision);
            uart_send_string("\r\n");

            struct arm_memory mem_info = get_arm_memory();
            uart_send_string("ARM memory:\r\n");
            uart_send_string("  base address: ");
            uart_send_hex(mem_info.base_addr);
            uart_send_string("\r\n");
            uart_send_string("  size        : ");
            uart_send_hex(mem_info.size);
            uart_send_string("\r\n");
        } else {
            uart_send_string("Command not found\r\n");
            uart_send_string("Use \"help\" to see help \r\n");
        }
    }
}