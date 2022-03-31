#include <printf.h>
#include <mini_uart.h>
#include <util.h>
#include <timer.h>

void exception_entry(unsigned long spsr, unsigned long elr, unsigned long esr) 
{
    printf("spsr_el1\t%x\r\n", spsr);
    printf("elr_el1\t\t%x\r\n", elr);
    printf("esr_el1\t\t%x\r\n\n", esr);

    return;
}

void lower_el_one_irq_handler()
{
    core_timer_handler();
    return;
}

void current_sp_elx_irq_handler()
{   
    if (get(CORE0_IRQ_SRC) & 0b10) {
        each_timer_handler();
    } else if (!(get(AUX_MU_IIR_REG) & 1)) {
        uart_handler();
    }
}