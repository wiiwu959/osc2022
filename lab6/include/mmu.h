#ifndef _MMU_H
#define _MMU_H

#define PGD_PAGE_FRAME 0x1000
#define PUD_PAGE_FRAME 0x2000

#define PHYS_OFFSET 0xffff000000000000

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE_nGnRnE 0
#define MAIR_IDX_NORMAL_NOCACHE 1

#define MAIR_CONFIG_DEFAULT \
    ((MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) |   \
    (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)))

#define PD_TABLE 0b11
#define PD_BLOCK 0b01
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR PD_TABLE
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE_nGnRnE << 2) | PD_BLOCK)

#define PD_USER_RW   (0b01 << 6)
#define PD_USER_RO   (0b11 << 6)
#define PD_USER_NRW  (0b00 << 6)

// The unprivileged execute-never bit, non-executable page frame for EL0 if set.
#define PD_USER_X   (0 << 54)
#define PD_USER_NX  (1 << 54)

// The privileged execute-never bit, non-executable page frame for EL1 if set.
#define PD_PXN (1 << 53)

#include <stdint.h>

void mmu_init();

void kernel_space_mapping();

uint64_t physical_to_virtual(uint64_t physical);
uint64_t virtual_to_physical(uint64_t virtual);

void map_pages(uint64_t* pagetable, uint64_t va, uint64_t size, uint64_t pa, uint64_t flag);

#endif  /*_MMU_H */