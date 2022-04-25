// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 8
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm

.global from_el2_to_el1
from_el2_to_el1:
	mov x10, (1 << 31)			// EL1 uses aarch64
	msr hcr_el2, x10
	mov x10, 0x3c5				// EL1h (SPSel = 1) with interrupt disabled
	msr spsr_el2, x10
	msr elr_el2, lr
	eret						// return to EL1

.global get_el
get_el:
	mrs x0, CurrentEL
	lsr x0, x0, #2
	ret

.global enable_interrupt
enable_interrupt:
	msr daifclr, 0xf 
	ret

.global disable_interrupt
disable_interrupt:
	msr	daifset, 0xf
	ret

.global disable_irq_save
disable_irq_save:
    mrs x0, DAIF
    msr DAIFSet, 0x2
    ret

.global irq_restore
irq_restore:
    msr DAIF, x0
    ret

exception_handler:
    save_all
    mrs x0, spsr_el1
    mrs x1, elr_el1
    mrs x2, esr_el1
    bl exception_entry
    load_all
    eret

_lower_el_one_irq:
    save_all
    bl lower_el_one_irq_handler
    load_all
    eret

_current_sp_elx_irq:
    save_all
    bl current_sp_elx_irq_handler
    load_all
    eret

_lower_el_one_sync:
    save_all
    bl lower_el_one_sync_handler
    load_all
    eret

.section ".vector"
.global exception_vector_table

// vector table should be aligned to 0x800
.align 11       
// branch to a handler function.
// entry size is 0x80, .align will pad 0
exception_vector_table:
// Exception from the current EL while using SP_EL0
current_sp_el0_sync:
    b exception_handler
.align 7
current_sp_el0_irq:
    b exception_handler
.align 7
current_sp_el0_fiq:
    b exception_handler
.align 7
current_sp_el0_serror:
    b exception_handler

// Exception from the current EL while using SP_ELx
.align 7
current_sp_elx_sync:
    b exception_handler
.align 7
current_sp_elx_irq:
    b _current_sp_elx_irq
.align 7
current_sp_elx_fiq:
    b exception_handler
.align 7
current_sp_elx_serror:
    b exception_handler

// Exception from a lower EL and at least one lower ELs are AArch32
.align 7
lower_el_one_sync:
    b _lower_el_one_sync
.align 7
lower_el_one_irq:
    b _lower_el_one_irq
.align 7
lower_el_one_fiq:
    b exception_handler
.align 7
lower_el_one_serror:
    b exception_handler

// Exception from a lower EL and all lower ELs are AArch32
.align 7
lower_el_all_sync:
    b exception_handler
.align 7
lower_el_all_irq:
    b exception_handler
.align 7 
lower_el_all_fiq:
    b exception_handler
.align 7
lower_el_all_serror:
    b exception_handler

.align 7
