#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stddef.h>
#include <list.h>

#define MEM_HEAD (char*)0x9000000
#define MEM_TAIL (char*)0x9ffffff


#define ALLOCATED 1
#define FREE_PAGE 0

#define PAGE_BASE 0x10000000
#define PAGE_END 0x20000000

#define PAGE_SIZE 0x1000
#define PAGE_TOTAL ((PAGE_END - PAGE_BASE) / PAGE_SIZE)

// free list max page count
#define PAGE_MAX_FREE 16

typedef struct {
    int status;
    int val;
} page_head;


void page_init(void);
void page_test(void);

void* simple_malloc(size_t size);

#endif  /*_ALLOCATOR_H */