#include <printf.h>
#include <mini_uart.h>
#include <util.h>
#include <timer.h>
#include <allocator.h>
#include <exception.h>

#define WAITING 0
#define RUNNING 1

typedef struct _task {
    int status;
    int priority;
    void (*callback)(void);
    struct _task *next;
} task;

task* task_list_head;

void init_task_list()
{
    task_list_head = NULL;
}

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
    // printf("spsr_el1\t%x\r\n", spsr);
    // printf("elr_el1\t\t%x\r\n", elr);
    // printf("esr_el1\t\t%x\r\n\n", esr);

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
        add_task(each_timer_handler, 1);
        // each_timer_handler();
    } else if (!(get(AUX_MU_IIR_REG) & 1)) {
        add_task(uart_handler, 0);
        // uart_handler();
    }
}
