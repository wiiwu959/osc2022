#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stddef.h>
#include <list.h>
#include <stdint.h>

#define MEM_HEAD (char*)(PHYS_OFFSET + 0x9000000)
#define MEM_TAIL (char*)(PHYS_OFFSET + 0x9ffffff)

void* simple_malloc(size_t size);

#define FREE_PAGE 0
#define ALLOCATED 1
#define IS_CHUNK 2

// #define PAGE_BASE (intptr_t)0x10000000
// #define PAGE_END (intptr_t)0x20000000

#define PAGE_SIZE 0x1000
// #define PAGE_TOTAL ((PAGE_END - PAGE_BASE) / PAGE_SIZE)

uint64_t page_base;
uint64_t page_end;
uint64_t page_total;

// free list max page count
#define PAGE_MAX_FREE 16
#define CHUNK_MAX_FREE 8

typedef struct {
    int status;
    int val;
} page_head;

void mm_init(char* fdt);
void page_init(void);
void chunk_init(void);

void* kmalloc(int size);
void kfree(void *ptr);

void page_test(void);
void mem_test(void);

static inline int addr2index(struct list_head* ptr) {
    return ((char*)(ptr) - (char*)page_base) / PAGE_SIZE;
}

static inline void* index2addr(int num) {
    return (void*)(page_base + num * PAGE_SIZE);
}

#endif  /*_ALLOCATOR_H */