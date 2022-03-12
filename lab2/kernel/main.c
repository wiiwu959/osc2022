#include <mini_uart.h>
#include <reboot.h>
#include <mailbox.h>
#include <string.h>

char buf[0x200];

void cmd_help(void)
{
    uart_sendline("help     :print this help menu\r\n");
    uart_sendline("hello    :print Hello World!\r\n");
    uart_sendline("reboot   :reboot the device\r\n");
    uart_sendline("hardware :get hardware's info\r\n");
    uart_sendline("ls       :list files\r\n");
    uart_sendline("cat      :capture file content\r\n");
}

void cmd_hello(void)
{
    uart_sendline("Hello World!\r\n");
}

void cmd_reboot(void)
{
    uart_sendline("Rebooting...\r\n");
    uart_reset(5);
}

void cmd_hardware(void)
{
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
}

void cmd_ls()
{
    cpio_list();
}

void cmd_cat()
{
    uart_sendline("Filename: ");
    uart_recvline(buf);
    uart_sendline("\r\n");
    cpio_cat(buf);
}

void cmd_hint(void)
{
    uart_sendline("Command not found\r\n");
    uart_sendline("Use \"help\" to see help \r\n");
}


void shell(void)
{
    while (1) {
        uart_sendline("# ");
        uart_recvline(buf);
        uart_sendline("\r\n");

        if (!strcmp("help", buf)) {
            cmd_help();
        } else if (!strcmp("hello", buf)) {
            cmd_hello();
        } else if (!strcmp("reboot", buf)) {
            cmd_reboot();
        } else if (!strcmp("hardware", buf)) {
            cmd_hardware();
        } else if (!strcmp("ls", buf)) {
            cmd_ls();
        } else if (!strcmp("cat", buf)) {
            cmd_cat();
        } else {
            cmd_hint();
        }
    }
}

void main(void)
{
    uart_init();
    uart_sendline("Hello! Type command to start.\r\n");
    shell();
}