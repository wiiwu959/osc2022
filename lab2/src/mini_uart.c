#include <mini_uart.h>
#include <util.h>

void uart_init ( void )
{
    // Initialization
    put(AUX_ENABLES, 1);        // Enable mini UART, Then mini UART register can be accessed
    put(AUX_MU_CNTL_REG, 0);    // Disable transmitter and receiver during configuration
    put(AUX_MU_IER_REG, 0);     // Disable interrupt because currently you don’t need interrupt
    put(AUX_MU_LCR_REG, 3);     // Set the data size to 8 bit
    put(AUX_MU_MCR_REG, 0);     // Don’t need auto flow control
    put(AUX_MU_BAUD_REG, 270);  // Set baud rate to 115200
    put(AUX_MU_IIR_REG, 6);     // No FIFO
    put(AUX_MU_CNTL_REG, 3);    // Enable the transmitter and receiver

    unsigned int selector;
    selector = get(GPFSEL1);
    selector &= ~(0b111 << 12); // clear 14-12 bits for GPIO14 
    selector |= (0b010 << 12);  // put 14-12 to 010 to take ALT0
    selector &= ~(0b111 << 15); // clear 17-15 bits for GPIO14 
    selector |= (0b010 << 15);  // put 17-15 to 010 to take ALT5
    put(GPFSEL1, selector);

    put(GPPUD, 0);
    delay(150);
    selector = 0b11 << 14;      // select 14, 15 bits
    put(GPPUDCLK0, selector);
    delay(150);
    put(GPPUDCLK0, 0);
}

char uart_recv(void)
{
    while (!(get(AUX_MU_LSR_REG) & 0x01)) {};
    return(get(AUX_MU_IO_REG) & 0xFF);
}

void uart_recvn(char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        while (!(get(AUX_MU_LSR_REG) & 0x01)) {};
        buf[i] = (get(AUX_MU_IO_REG) & 0xFF);
    }
}

void uart_recvline(char *buf)
{
    char c = uart_recv();
    while(c != '\r') {
        *buf = c;
        buf++;
        uart_send(c);
        c = uart_recv();
    }
    *buf = '\0';
}

void uart_send(char c)
{
    while (!(get(AUX_MU_LSR_REG) & 0x20)) {};
    put(AUX_MU_IO_REG, c);
}

void uart_sendline(char* str)
{
    while (*str != '\0') {
        uart_send(*str);
        str++;
    }
}

void uart_send_hex(unsigned int num)
{
    uart_sendline("0x");
    for (int i = 28; i >= 0; i -= 4) {
        char b = (char)(0xF & (num >> i));
        if ((int)b < 10) {
            uart_send(b + 48);
        } else {
            uart_send(b + 55);
        }
    }
}