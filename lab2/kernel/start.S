.section ".text.boot"
.global _start
_start: 
    mrs x10, mpidr_el1
    and x10, x10, #0xFF           // Check processor id
    cbnz x10, proc_hang           // Hang for all non-primary CPU

    ldr x10, =_sbss
    ldr x11, =_ebss
    sub x11, x11, x10             // x11 = bss segment

memzero:
    str xzr, [x10], #8            // *x10 = 0 then x10 = x10 + 8
    subs x11, x11, #8             // x11 = x11 - 8
    bgt memzero                   // compare cpsr register

    ldr x10, =_estack             // place stack pointer
    mov sp, x10 
    bl  main

proc_hang: 
    b   proc_hang