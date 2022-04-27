#include <printf.h>
#include <mini_uart.h>
#include <util.h>
#include <timer.h>
#include <allocator.h>
#include <exception.h>
#include <sched.h>
#include <sysregs.h>
#include <syscall.h>

#define WAITING 0
#define RUNNING 1

// TODO: Fix this shit
// Preemption ??? Not working now --- start
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
// Preemption ??? Not working now --- end

void exception_entry(unsigned long spsr, unsigned long elr, unsigned long esr) 
{
    printf("spsr_el1\t%x\r\n", spsr);
    printf("elr_el1\t\t%x\r\n", elr);
    printf("esr_el1\t\t%x\r\n\n", esr);

    disable_interrupt();
    while(1);
    return;
}

void lower_el_one_irq_handler()
{
    // core_timer_handler();
    // return;

    if (get(CORE0_IRQ_SRC) & 0b10) {
        // add_task(each_timer_handler, 1);
        each_timer_handler();
    } else if (!(get(AUX_MU_IIR_REG) & 1)) {
        // add_task(uart_handler, 0);
        uart_handler();
    }
}

void current_sp_elx_irq_handler()
{   
    if (get(CORE0_IRQ_SRC) & 0b10) {
        // add_task(each_timer_handler, 1);
        each_timer_handler();
    } else if (!(get(AUX_MU_IIR_REG) & 1)) {
        // add_task(uart_handler, 0);
        uart_handler();
    }
}

void lower_el_one_sync_handler(struct trap_frame *regs)
{
    enable_interrupt();
    unsigned long esr;
    asm volatile("mrs %0, esr_el1": "=r" (esr));
    if ((esr >> ESR_ELx_EC_SHIFT) == ESR_ELx_EC_SVC64) {
        syscall_handler(regs);
    } else {
        unsigned long spsr, elr;
        asm volatile("mrs %0, spsr_el1": "=r" (spsr));
        asm volatile("mrs %0, elr_el1": "=r" (elr));
        printf("spsr_el1\t%x\r\n", spsr);
        printf("elr_el1\t\t%x\r\n", elr);
        printf("esr_el1\t\t%x\r\n\n", esr);
        printf("Unknown lower_el_one_sync exception.\n");
        while (1) {}
    }
    disable_interrupt();
    // schedule();
}