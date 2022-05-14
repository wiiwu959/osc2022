#ifndef	_MINI_UART_H
#define	_MINI_UART_H

#define MMIO_BASE 0x3f000000

// ref: https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p.8
#define AUX_IRQ                 (MMIO_BASE+0x00215000)
#define AUX_ENABLES             (MMIO_BASE+0x00215004)
#define AUX_MU_IO_REG           (MMIO_BASE+0x00215040)
#define AUX_MU_IER_REG          (MMIO_BASE+0x00215044)
#define AUX_MU_IIR_REG          (MMIO_BASE+0x00215048)
#define AUX_MU_LCR_REG          (MMIO_BASE+0x0021504C)
#define AUX_MU_MCR_REG          (MMIO_BASE+0x00215050)
#define AUX_MU_LSR_REG          (MMIO_BASE+0x00215054)
#define AUX_MU_MSR_REG          (MMIO_BASE+0x00215058)
#define AUX_MU_SCRATCH          (MMIO_BASE+0x0021505C)
#define AUX_MU_CNTL_REG         (MMIO_BASE+0x00215060)
#define AUX_MU_STAT_REG         (MMIO_BASE+0x00215064)
#define AUX_MU_BAUD_REG         (MMIO_BASE+0x00215068)
#define AUX_SPI0_CNTL0_REG      (MMIO_BASE+0x00215080)
#define AUX_SPI0_CNTL1_REG      (MMIO_BASE+0x00215084)
#define AUX_SPI0_STAT_REG       (MMIO_BASE+0x00215088)
#define AUX_SPI0_IO_REG         (MMIO_BASE+0x00215090)
#define AUX_SPI0_PEEK_REG       (MMIO_BASE+0x00215094)
#define AUX_SPI1_CNTL0_REG      (MMIO_BASE+0x002150C0)
#define AUX_SPI1_CNTL1_REG      (MMIO_BASE+0x002150C4)
#define AUX_SPI1_STAT_REG       (MMIO_BASE+0x002150C8)
#define AUX_SPI1_IO_REG         (MMIO_BASE+0x002150D0)
#define AUX_SPI1_PEEK_REG       (MMIO_BASE+0x002150D4)

// ref: https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p.90
#define GPFSEL0                 (MMIO_BASE+0x00200000)
#define GPFSEL1                 (MMIO_BASE+0x00200004)
#define GPFSEL2                 (MMIO_BASE+0x00200008)
#define GPFSEL3                 (MMIO_BASE+0x0020000C)
#define GPFSEL4                 (MMIO_BASE+0x00200010)
#define GPFSEL5                 (MMIO_BASE+0x00200014)

#define GPPUD                   (MMIO_BASE+0x00200094)
#define GPPUDCLK0               (MMIO_BASE+0x00200098)
#define GPPUDCLK1               (MMIO_BASE+0x0020009C)

#define CORE0_TIMERS_ITR_CON    0x40000040
#define CORE0_IRQ_SRC           0x40000060

#define IRQ_PENDING_1           (MMIO_BASE+0x0000B204)
#define ENABLE_IRQs1            (MMIO_BASE+0x0000B210)


void uart_init(void);
char uart_recv(void);
void uart_recvn(char *buf, int len);
void uart_recvline(char* str);
void uart_send(char c);
void uart_sendn(char *str, int len);
void uart_send_string(char* str);
void uart_send_hex(unsigned int num);

// interrupt buffer
#define BUFFER_MAX_SIZE 0x100

void enable_uart_interrupt(void);
void disable_uart_interrupt(void);

void uart_handler();
char uart_async_recv();
void uart_async_recvn(char* buf, int len);
void uart_async_recvline(char *buf);
void uart_async_send(char c);
void uart_async_send_string(char *str);

#endif  /*_MINI_UART_H */