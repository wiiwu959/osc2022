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

uint64_t initramfs_loc;
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
    uart_send_string("malloc          :testing simple_malloc\r\n");
    uart_send_string("set <sec> <msg> :set a timer and print msg\r\n");
    uart_send_string("timer           :set 10 timers\r\n");
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
    cpio_list(initramfs_loc);
}

void cmd_cat()
{
    uart_send_string("Filename: ");
    uart_recvline(buf);
    uart_send_string("\r\n");
    cpio_cat(buf, initramfs_loc);
}

void cmd_load()
{
    uart_send_string("Filename: ");
    uart_recvline(buf);
    uart_send_string("\r\n");
    cpio_exec(buf, initramfs_loc);
}

void cmd_malloc()
{
    char *meow = simple_malloc(sizeof("meow"));
    char *cover = simple_malloc(sizeof("kitty\0"));

    meow[0] = 'm';
    meow[1] = 'e';
    meow[2] = 'o';
    meow[3] = 'w';
    meow[4] = '?';
    
    uart_send_string("Address of meow[5] : ");
    uart_send_hex((unsigned int)&meow[5]);
    uart_send_string("\r\n");
    uart_send_string("Address of cover   : ");
    uart_send_hex((unsigned int)cover);
    uart_send_string("\r\n");
    
    meow[5] = 'k';
    meow[6] = 'i';
    meow[7] = 't';
    meow[8] = 't';
    meow[9] = 'y';
    meow[10] = '\0';
    
    uart_send_string(meow);
    uart_send_string("\r\n");
    uart_send_string(cover);
    uart_send_string("\r\n");
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
    add_timer(print_msg, sec, arg[2]);
}

void cmd_timer()
{
    add_timer(print_msg, 1, "timer 1");
    add_timer(print_msg, 2, "timer 2");
    add_timer(print_msg, 3, "timer 3");
    add_timer(print_msg, 4, "timer 4");
    add_timer(print_msg, 5, "timer 5");
    add_timer(print_msg, 6, "timer 6");
    add_timer(print_msg, 7, "timer 7");
    add_timer(print_msg, 8, "timer 8");
    add_timer(print_msg, 9, "timer 9");
    add_timer(print_msg, 10, "timer 10");
}

void cmd_hint()
{
    uart_send_string("Command not found\r\n");
    uart_send_string("Use \"help\" to see help \r\n");
}

void initramfs_callback(char* fdt, char* node) 
{
    char* cur = node;
    cur += fdt_alignup(strlen(node) + 1, 4);
    char* dt_string = fdt_get_dt_string(fdt);

    while (fdt_get_uint32(cur) == FDT_PROP) {
        cur += 4;
        struct fdt_prop* prop = (struct fdt_prop*)cur;
        uint32_t prop_len = fdt_get_uint32((char*)&prop->len);
        char* prop_name = dt_string + fdt_get_uint32((char*)&prop->nameoff);

        cur += sizeof(struct fdt_prop);
        if(!strcmp(prop_name, "linux,initrd-start")) {
            uart_send_string("[*] cpio archive file loaded at: ");
            initramfs_loc = fdt_get_uint32(cur);
            uart_send_hex(initramfs_loc);
            uart_send_string("\r\n");
        }
        cur += fdt_alignup(prop_len, 4);
    }
    return;
}

void initramfs_init(char* fdt)
{
    initramfs_loc = 0;
    fdt_traverse(fdt, initramfs_callback);
}

void shell(void)
{
    while (1) {        
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
        } else if (!strcmp("malloc", buf)) {
            cmd_malloc();
        } else if (!strcmp("async", buf)) {
            cmd_async();
        } else if (!strncmp("set", buf, 3)) {
            cmd_settimeout(buf);
        } else if (!strcmp("timer", buf)) {
            cmd_timer();
        } else {
            cmd_hint();
        }
    }
}