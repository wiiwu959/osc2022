.section ".text.boot"
.global _start
_start: 
    ldr x10, =_bootloader
    ldr x11, =_estack
    sub x11, x11, x10              // x11 = bootloader_size
    
    adr x12, _start                // x12 = real start location (0x80000)
    add x11, x11, x12              // x11 = real end address of bootloader

relocate:
    cmp x12, x11
    bgt relocate_finish

    ldr x15, [x12]
    str x15, [x10]
    add x10, x10, #8
    add x12, x12, #8
    b   relocate
    
relocate_finish:
    ldr x10, =_bootloader
    adr x11, _handle_cpu
    adr x12, _start
    sub x11, x11, x12              // x11 = offset from _start to _handle_cpu
    add x10, x10, x11
    be  x10                        // jump to relocattion address and from _handle_cpu

_handle_cpu:
    mrs x10, mpidr_el1
    and x10, x10, #0xFF            // Check processor id
    cbnz x10, proc_hang            // Hang for all non-primary CPU

    ldr x10, =_sbss
    ldr x11, =_ebss
    sub x11, x11, x10              // x11 = bss segment

memzero:
    str xzr, [x10], #8             // *x10 = 0 then x10 = x10 + 8
    subs x11, x11, #8              // x11 = x11 - 8
    bgt memzero                    // compare cpsr register

    ldr x10, =_estack              // place stack pointer
    mov sp, x10 
    bl  bootloader_main

proc_hang: 
    b   proc_hang