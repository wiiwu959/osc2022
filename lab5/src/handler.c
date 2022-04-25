#include <printf.h>
#include <mini_uart.h>
#include <util.h>
#include <timer.h>
#include <allocator.h>
#include <exception.h>
#include <sched.h>

#define WAITING 0
#define RUNNING 1

typedef struct _task {
    int status;
    int priority;
    void (*callback)(void);
    struct _task *next;
} task;

task* task_list_head = NULL;

void exec_task()
{
    while (task_list_head != NULL) {
        task_list_head->status = RUNNING;
        disable_interrupt();
        task_list_head->callback();
        enable_interrupt();
        // kfree(task_list_head);
        task_list_head = task_list_head->next;
    }
}

void add_task(void (*callback)(void), int priority)
{
    // task* add = kmalloc(sizeof(task));
    task* add = simple_malloc(sizeof(task));
    add->callback = callback;
    add->priority = priority;
    add->status = WAITING;
    if (task_list_head == NULL || task_list_head->priority > priority) {
        add->next = task_list_head;
        task_list_head = add;
        exec_task();
    } else {
        task* cur = task_list_head;
        while (cur->next != NULL && cur->next->priority <= priority) {
            cur = cur->next;
        }
        add->next = cur->next;
        cur->next = add;
    }
}
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
        // add_task(each_timer_handler, 1);
        each_timer_handler();
    } else if (!(get(AUX_MU_IIR_REG) & 1)) {
        // add_task(uart_handler, 0);
        // printf("uart");
        uart_handler();
    }
}

#include <sched.h>
#define ESR_ELx_EC(esr)	((esr & 0xFC000000) >> 26)
#define ESR_ELx_ISS(esr)	(esr & 0x03FFFFFF)

#define ESR_ELx_EC_SVC64 0b010101
#define ESR_ELx_EC_DABT_LOW 0b100100
#define ESR_ELx_EC_IABT_LOW 0b100000
#define ESR_ELx_EC_BRK_LOW 0b110000

#define read_sysreg(r) ({                       \
    unsigned long __val;                        \
    asm volatile("mrs %0, " #r : "=r" (__val)); \
    __val;                                      \
})

#define write_sysreg(r, __val) ({                  \
	asm volatile("msr " #r ", %0" :: "r" (__val)); \
})

struct pt_regs {
    unsigned long regs[31];
    unsigned long sp;
    unsigned long pc;
    unsigned long pstate;
};


void lower_el_one_sync_handler(struct pt_regs* regs)
{
    unsigned long esr = read_sysreg(esr_el1);
    unsigned ec = ESR_ELx_EC(esr);
    unsigned iss = ESR_ELx_ISS(esr);

    enable_interrupt();

    switch (ec) {
    case ESR_ELx_EC_SVC64:
        /* iss[24-16] = res0  */
        /* iss[15-0]  = imm16 */
        // if ((iss & 0xffff) == 0) {
        //     svc_handler(regs);
        // }
        printf("ESR_ELx_EC_SVC64\r\n");
        break;

    case ESR_ELx_EC_DABT_LOW:
        /* Userland data abort exception */
        printf("ESR_ELx_EC_DABT_LOW\r\n");
        break;

    case ESR_ELx_EC_IABT_LOW:
        /* Userland instruction abort exception */
        printf("ESR_ELx_EC_IABT_LOW\r\n");
        break;

    case ESR_ELx_EC_BRK_LOW:
        printf("Breakpoint exception\r\n");

    default:
        printf("Unknown exception: EC=0x%x, ISS=0x%x\r\n", ec, iss);
    }

    schedule();
    // disable_interrupt();
    return;
}
