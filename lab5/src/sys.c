#include <sys.h>
#include <stddef.h>
#include <mini_uart.h>
#include <sched.h>
#include <mailbox.h>

int sys_getpid()
{
    return get_current()->pid;
}


size_t sys_uartread(char buf[], size_t size)
{
    for (size_t i = 0; i < size; i++) {
        char c = uart_recv();
        buf[i] = c;
    }
    return size;
}

size_t sys_uartwrite(const char buf[], size_t size)
{
    for (size_t i = 0; i < size; i++) {
        uart_send(buf[i]);
    }
    return size;
}

int sys_exec(const char *name, char *const argv[])
{

}

int sys_fork()
{

}

void sys_exit(int status)
{

}

int sys_mbox_call(unsigned char ch, unsigned int *mbox)
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

void sys_kill(int pid)
{

}
