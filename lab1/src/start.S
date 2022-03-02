.section ".text.boot"
.global _start
_start: 
    mrs x0, mpidr_el1
    and x0, x0, #0xFF    // Check processor id
    cbnz    x0, proc_hang      // Hang for all non-primary CPU

    ldr x0, =_sbss
    ldr x1, =_ebss
    sub x1, x1, x0

memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt memzero

    ldr x0, =_estack
    mov sp, x0 
    bl  main

proc_hang: 
    b   proc_hang

// https://www.keil.com/support/man/docs/armasm/armasm_dom1359731147386.htm