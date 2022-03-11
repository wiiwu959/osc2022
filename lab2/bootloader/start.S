.section ".text.boot"
.global _start
_start: 
    mrs x0, mpidr_el1
    and x0, x0, #0xFF           // Check processor id
    cbnz    x0, proc_hang       // Hang for all non-primary CPU

    ldr x0, =_sbss
    ldr x1, =_ebss
    sub x1, x1, x0              // x1 = bss segment

memzero:
    str xzr, [x0], #8          // *x0 = 0 then x0 = x0 + 8
    subs x1, x1, #8            // x1 = x1 - 8
    bgt memzero                // compare cpsr register

    ldr x0, =_estack           // place stack pointer
    mov sp, x0 
    bl  bootloader_main

proc_hang: 
    b   proc_hang