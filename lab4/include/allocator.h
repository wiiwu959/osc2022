#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stddef.h>
#include <list.h>

#define MEM_HEAD (char*)0x9000000
#define MEM_TAIL (char*)0x9ffffff

#define FREE_PAGE 0
#define ALLOCATED 1
#define IS_CHUNK 2

#define PAGE_BASE 0x10000000
#define PAGE_END 0x20000000

#define PAGE_SIZE 0x1000
#define PAGE_TOTAL ((PAGE_END - PAGE_BASE) / PAGE_SIZE)

// free list max page count
#define PAGE_MAX_FREE 16
#define CHUNK_MAX_FREE 8

typedef struct {
    int status;
    int val;
} page_head;

void page_init(void);

void page_test(void);
void mem_test(void);

void* kmalloc(int size);
void kfree(void *ptr);

void* simple_malloc(size_t size);

#endif  /*_ALLOCATOR_H */