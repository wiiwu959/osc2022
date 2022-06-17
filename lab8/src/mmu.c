#include <mmu.h>
#include <allocator.h>
#include <stddef.h>
#include <mm.h>

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


void kernel_space_mapping()
{
    // 2MB block mapping
    uint64_t *p1 = kmalloc(PAGE_SIZE);
    
    // 0x00000000 - 0x3e800000
    for (int i = 0; i < 500; i++) {
        p1[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_BLOCK;
    }
    // 0x3ea00000 - 0x3fffffff
    for (int i = 500; i < 512; i++) {
        p1[i] = (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
    }

    uint64_t *p2 = kmalloc(PAGE_SIZE);
    // 0x40000000 - 0x7fffffff
    for (int i = 0; i < 512; i++) {
        p2[i] = 0x40000000 | (i << 21) | PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK;
    }

    // 1st 2MB mapped by the 1st entry of PUD
    asm volatile("str %0, [%1]\r\n" :: "r"((uint64_t)virtual_to_physical(p1) | PD_TABLE),
                                       "r"(physical_to_virtual(PUD_PAGE_FRAME)));
    // 2nd 2MB mapped by the 2nd entry of PUD
    asm volatile("str %0, [%1]\r\n" :: "r"((uint64_t)virtual_to_physical(p2) | PD_TABLE),
                                       "r"(physical_to_virtual(PUD_PAGE_FRAME + 8)));
}

uint64_t physical_to_virtual(uint64_t physical)
{
    return (physical | 0xffff000000000000);
}

uint64_t virtual_to_physical(uint64_t virtual)
{
    return (virtual & 0x0000ffffffffffff);
}

// PGD -> PUD -> PMD -> PTE
void _map_pages(uint64_t* pagetable, uint64_t va, uint64_t pa, uint64_t flag)
{
    uint64_t pd;
    for (int level = 3; level > 0; level--) {
        int index = ((uint64_t)(va) >> (12 + 9 * level)) & 0b111111111;
        pd = pagetable[index];

        // page table not exist
        if (!(pd & 1)) {
            uint64_t *tmp = kmalloc(PAGE_SIZE);
            memset(tmp, 0, PAGE_SIZE);
            pagetable[index] = virtual_to_physical(tmp) | PD_TABLE;
            pagetable = tmp;
        } else {
            // page table existed
            pagetable = (uint64_t *)physical_to_virtual(pd & ~((uint64_t)0xfff));
        }
    }

    int index = ((uint64_t)(va) >> 12) & 0b111111111;
    pagetable[index] = (uint64_t)(pa)  | flag | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_PXN | PD_ACCESS | PD_TABLE;
}

void map_pages(uint64_t* pagetable, uint64_t va, uint64_t size, uint64_t pa, uint64_t flag)
{
    // align address first
    size = ((size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
    for (uint64_t i = 0; i < size; i += PAGE_SIZE) {
        _map_pages(pagetable, va + i, pa + i, flag);
    }
}