#include <reboot.h>
#include <mini_uart.h>
#include <mailbox.h>
#include <cpio.h>
#include <allocator.h>
#include <string.h>
#include <allocator.h>
#include <fdt.h>
#include <timer.h>
#include <printf.h>
#include <sched.h>
#include <exec.h>

static char buf[0x200];

void cmd_help(void)
{
    uart_send_string("help            :print this help menu\r\n");
    uart_send_string("hello           :print Hello World!\r\n");
    uart_send_string("reboot          :reboot the device\r\n");
    uart_send_string("hardware        :get hardware's info\r\n");
    uart_send_string("ls              :list files\r\n");
    uart_send_string("cat             :capture file content\r\n");
    uart_send_string("load            :load a executable file\r\n");
    uart_send_string("set <sec> <msg> :set a timer and print msg\r\n");
    uart_send_string("timer           :set 10 timers\r\n");
    uart_send_string("memtest         :testing kmalloc\r\n");
    uart_send_string("pagetest        :testing page_alloc\r\n");
    uart_send_string("async           :testing uart_async\r\n");
}

void cmd_hello(void)
{
    uart_send_string("Hello World!\r\n");
}

void cmd_reboot(void)
{
    uart_send_string("Rebooting...\r\n");
    uart_reset(5);
}

void cmd_hardware(void)
{
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
}

void cmd_ls()
{
    cpio_list();
}

void cmd_cat()
{
    uart_send_string("Filename: ");
    uart_recvline(buf);
    uart_send_string("\r\n");
    cpio_cat(buf);
}

void cmd_load()
{
    uart_send_string("Filename: ");
    uart_recvline(buf);
    uart_send_string("\r\n");
    exec_program(buf);
}

void cmd_async()
{
    // enable_interrupt();
    enable_uart_interrupt();

    char buffer[BUFFER_MAX_SIZE];
    int i = 0;
    char c = uart_async_recv();
    while (c != '\n') {
        buffer[i] = c;
        c = uart_async_recv();
        i++;
    }
    buffer[i + 1] = '\0';
    uart_async_send_string(buffer);
    uart_async_send_string("\r\n");

    disable_uart_interrupt();
    // disable_interrupt();
}

void print_msg(char* msg)
{
    uart_send_string("[*] message: ");
    uart_send_string(msg);
    uart_send_string("\r\n");
}

void cmd_settimeout(char* buffer)
{
    int arg_num = 0;
    char* arg[BUFFER_MAX_SIZE];
    arg[arg_num] = buffer;
    while (*buffer) {
        if (*buffer == ' ') {
            *buffer = '\0';
            arg[++arg_num] = buffer + 1;
        }
        buffer++;
    }

    int sec = atoi(arg[1]);
    timer_add(print_msg, sec, arg[2]);
}

void cmd_timer()
{
    timer_add(print_msg, 1, "timer 1");
    timer_add(print_msg, 2, "timer 2");
    timer_add(print_msg, 3, "timer 3");
    timer_add(print_msg, 4, "timer 4");
    timer_add(print_msg, 5, "timer 5");
    timer_add(print_msg, 6, "timer 6");
    timer_add(print_msg, 7, "timer 7");
    timer_add(print_msg, 8, "timer 8");
    timer_add(print_msg, 9, "timer 9");
    timer_add(print_msg, 10, "timer 10");
}

void fooo()
{
    for(int i = 0; i < 10; ++i) {
        printf("Thread id: %d %d\n", get_current()->pid, i);
        delay(1000000);
        schedule();
    }
}

void cmd_testfoo()
{
    for (int i = 0; i < 5; i++) {
        kthread_create(&fooo, "foooo");    
    }
    printf("lkmomonononoinonoinonin???\r\n");
}

void cmd_hint()
{
    uart_send_string("Command not found\r\n");
    uart_send_string("Use \"help\" to see help \r\n");
}

#include <exception.h>
void shell(void)
{
    uart_send_string("Hello! Type command to start.\r\n");
    while (1) {
        // TODO: Find out WTF happenning to this shit
        enable_interrupt();
        uart_send_string("# ");

        uart_recvline(buf);
        uart_send_string("\r\n");

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
        } else if (!strcmp("load", buf)) {
            cmd_load();
        } else if (!strcmp("async", buf)) {
            cmd_async();
        } else if (!strncmp("set", buf, 3)) {
            cmd_settimeout(buf);
        } else if (!strcmp("timer", buf)) {
            cmd_timer();
        } else if (!strcmp("memtest", buf)) {
            mem_test();
        } else if (!strcmp("pagetest", buf)) {
            page_test();
        } else if (!strcmp("testfoo", buf)) {
            cmd_testfoo();
        } else {
            cmd_hint();
        }
    }
}