#include <mmu.h>

void mmu_init()
{
    // set up tcr_el1
    asm volatile("msr tcr_el1, %0\r\n" :: "r"(TCR_CONFIG_DEFAULT));

    // set up mair_el1
    asm volatile("msr mair_el1, %0\r\n" :: "r"(MAIR_CONFIG_DEFAULT));

    // set up identity paging
    // combine the physical address of next level page with attribute
    asm volatile("str %0, [%1]\r\n" :: "r"(PUD_PAGE_FRAME | BOOT_PGD_ATTR),
                                       "r"(PGD_PAGE_FRAME));

    // 1st 1GB mapped by the 1st entry of PUD
    asm volatile("str %0, [%1]\r\n" :: "r"(0x00000000 | BOOT_PUD_ATTR),
                                       "r"(PUD_PAGE_FRAME));
    // 2nd 1GB mapped by the 2nd entry of PUD
    asm volatile("str %0, [%1]\r\n" :: "r"(0x40000000 | BOOT_PUD_ATTR),
                                       "r"(PUD_PAGE_FRAME + 8));

    // load PGD to the bottom translation-based register
    asm volatile("msr ttbr0_el1, %0\r\n" ::"r"(PGD_PAGE_FRAME));
    // also load PGD to the upper translation based register.
    asm volatile("msr ttbr1_el1, %0\r\n" ::"r"(PGD_PAGE_FRAME));
    
    // enable MMU, cache remains disabled
    asm volatile (
        "mrs x2, sctlr_el1\r\n"
        "orr x2 , x2, 1\r\n"
        "msr sctlr_el1, x2\r\n"
    );
}

void* physical_to_virtual(unsigned long long physical)
{
    return (void*)(physical + PHYS_OFFSET);
}

void* virtual_to_physical(unsigned long long virtual)
{
    return (void*)(virtual - PHYS_OFFSET);
}