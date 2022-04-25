#include <allocator.h>

#include <stddef.h>
#include <stdint.h>

#include <list.h>
#include <printf.h>
#include <mini_uart.h>
#include <cpio.h>

// #define DEBUG

char* malloc_ptr = MEM_HEAD;

struct list_head frame_freelist[PAGE_MAX_FREE];
page_head* frame_array;

int chunk_size[CHUNK_MAX_FREE] = {0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800};
struct list_head chunk_freelist[CHUNK_MAX_FREE];
int chunk_free_num[CHUNK_MAX_FREE];

void* simple_malloc(size_t size)
{
    char* ptr = malloc_ptr;
    if (malloc_ptr + size > MEM_TAIL) {
        return NULL;
    }
    ptr = malloc_ptr;
    malloc_ptr += size;
    return ptr;
}

void memory_reserve(void* from, void* to)
{
    from = (void*)((intptr_t)from & ~(PAGE_SIZE - 1));
    to = (void*)(((intptr_t)to + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
    for (intptr_t i = (intptr_t)from; i < (intptr_t)to; i += PAGE_SIZE) {
        int index = addr2index((void*)i);
        frame_array[index].status = ALLOCATED;
        frame_array[index].val = 0; // 2^0 = 1 (1 page)
    }
}

void mm_init(char* fdt)
{
    initramfs_init(fdt);
    page_base = (uint64_t)0;
    page_end = (uint64_t)0x3c000000;

    page_total = (page_end - page_base) / PAGE_SIZE;
    frame_array = simple_malloc(sizeof(page_head) * page_total);

    for (int i = 0; i < page_total; i++) {
        frame_array[i].val = 0;
        frame_array[i].status = FREE_PAGE;
    }

    //Spin tables for multicore boot
    memory_reserve((void*)0, (void*)0x1000); 
    
    // Kernel image in the physical memory
    memory_reserve((void*)0x80000, (void*)0x400000); 

    // Initramfs
    memory_reserve((uintptr_t)initramfs_loc, (uintptr_t)initramfs_end);

    // Devicetree
    memory_reserve((uintptr_t)fdt, fdt_end);

    // simple simple_allocator
    memory_reserve((uintptr_t)MEM_HEAD, (uintptr_t)MEM_TAIL);
    
    page_init();
    chunk_init();
}

void page_init()
{
    // init free list
    for (int i = 0; i < PAGE_MAX_FREE; i++) {
        INIT_LIST_HEAD(&frame_freelist[i]);
    }

    for (int exp = 0; exp < PAGE_MAX_FREE; exp++) {
        int index = 0;
        while (index < page_total) {
            int buddy_index = index ^ (1 << exp);
            if (buddy_index >= page_total) { break; }
            if(frame_array[index].status == FREE_PAGE && frame_array[index].val == exp) {
                if (frame_array[buddy_index].status == FREE_PAGE && frame_array[buddy_index].val == exp) {
                    frame_array[index].val = exp + 1;
                }
            }
            index += (1 << (exp + 1));
        }
    }

    // page free_list manage
    int index = 0;
    while (index < page_total) {
        if (frame_array[index].status == FREE_PAGE) {
            int exp = frame_array[index].val;
            struct list_head * added;
            added = (struct list_head *)index2addr(index);
            list_add_tail(added, &frame_freelist[exp]);
            index += 1 << exp;
        } else {
            index++;
        }
    }
}

void *alloc_page(int num)
{
    int target_exp = num - 1 ? sizeof(num - 1) * 8 - __builtin_clz(num - 1) : 0;

    for (int available = target_exp; available < PAGE_MAX_FREE; available++) {
        if (!list_empty(&frame_freelist[available])) {
            struct list_head* cur = frame_freelist[available].next;
            list_del(cur);

            for (int i = available; i > target_exp; i--) {
                struct list_head* add = (struct list_head*)((char*)cur + (PAGE_SIZE * (1 << (i - 1))));
                list_add(add, &frame_freelist[i - 1]);
                frame_array[addr2index(add)].val = i - 1;
                frame_array[addr2index(add)].status = FREE_PAGE;
#ifdef DEBUG
                printf("[*] Split exp %d frame to exp %d frame. \r\n", i, i - 1);
#endif
            }

            int index = addr2index(cur);
            frame_array[index].val = target_exp;
            frame_array[index].status = ALLOCATED;
#ifdef DEBUG
            printf("[*] Allocate %d pages using exp %d, index is = %d. \r\n", num, target_exp, index);
#endif
            return cur;
        }
    }
#ifdef DEBUG
    printf("[*] Not allocating any page. \r\n");
#endif
    return NULL;
}

void free_page(struct list_head* target) {
    int index = addr2index(target);

#ifdef DEBUG
    printf("[*] Free index = %d. \r\n", index);
#endif

    if (frame_array[index].status == FREE_PAGE) {
        printf("[*] Double free. \r\n");
        return;
    }

    int exp = frame_array[index].val;
    int buddy_index = index ^ (1 << exp);

    while (exp + 1 < PAGE_MAX_FREE &&
        frame_array[buddy_index].status == FREE_PAGE &&
        frame_array[buddy_index].val == exp) {
        
        struct list_head* buddy = (struct list_head*)index2addr(buddy_index);
        list_del(buddy);

#ifdef DEBUG
        printf("[*] Merge index %d with %d, back to free list exp %d. \r\n", index, buddy_index, exp + 1);
#endif
        exp++;
        if (buddy_index < index) {
            index = buddy_index;
            target = buddy;
        }
        buddy_index = index ^ (1 << exp);
    }

    list_add(target, &frame_freelist[exp]);
    frame_array[index].status = FREE_PAGE;
    frame_array[index].val = exp;
}

void chunk_init()
{
    for (int i = 0; i < CHUNK_MAX_FREE; i++) {
        INIT_LIST_HEAD(&chunk_freelist[i]);
        chunk_free_num[i] = 0;
    }
}

void* alloc_chunk(int size)
{
    int index;
    if (size <= 0x10) { index = 0; }
    else if (size <= 0x20) { index = 1; }
    else if (size <= 0x40) { index = 2; }  
    else if (size <= 0x80) { index = 3; } 
    else if (size <= 0x100) { index = 4; } 
    else if (size <= 0x200) { index = 5; }
    else if (size <= 0x400) { index = 6; }
    else if (size <= 0x800) { index = 7; }

    if (list_empty(&chunk_freelist[index])) {
        void* new_page = alloc_page(1);
        int frame_index = addr2index(new_page);
        frame_array[frame_index].status = IS_CHUNK;
        frame_array[frame_index].val = index;

        int page_size = PAGE_SIZE;
        while (page_size - size >= 0) {
            list_add_tail((struct list_head*)new_page, &chunk_freelist[index]);
            new_page = (char*)new_page + size;
            page_size -= size;
        }
    }
#ifdef DEBUG
    printf("[*] Allocate chunk size: %d \r\n", chunk_size[index]);
#endif
    struct list_head* ptr = chunk_freelist[index].next;
    list_del(ptr);
    return ptr;
}

void* kmalloc(int size)
{
    if (size <= 0x800) {
        return alloc_chunk(size);
    } else if (size <= 0x1000){
        return alloc_page(1);
    } else {
        int page_num = size / 0x1000;
        if ((size & 0xFFF) > 0) { page_num++; }
        return alloc_page(page_num);
    }
}

void kfree(void *ptr)
{
    int index = ((char*)((intptr_t)ptr & (~0xFFF)) - (char*)page_base) / PAGE_SIZE;
    if(frame_array[index].status == IS_CHUNK) {
        int chunk_index = frame_array[index].val;
        list_add_tail((struct list_head*)ptr, &chunk_freelist[chunk_index]);
#ifdef DEBUG
        printf("Free chunk of size %d. \r\n", chunk_size[chunk_index]);
#endif
    } else {
        free_page(ptr);
    }
}


// tests
void page_test(void)
{
    char* ptr1 = alloc_page(1);
    char* ptr2 = alloc_page(2);
    char* ptr3 = alloc_page(2);
    char* ptr4 = alloc_page(1);
    char* ptr5 = alloc_page(1);
    free_page(ptr1);
    free_page(ptr4);
    char* ptr6 = alloc_page(2);
    free_page(ptr2);
    free_page(ptr3);
    free_page(ptr5);
    free_page(ptr6);

    char* ptr7 = alloc_page(1);
    char* ptr8 = alloc_page(1);
    char* ptr9 = alloc_page(1);
    char* ptr10 = alloc_page(1);
    char* ptr11 = alloc_page(1);
    free_page(ptr7);
    free_page(ptr8);
    free_page(ptr9);
    free_page(ptr10);
    free_page(ptr11);

}

void mem_test() {
    char* ptr1 = kmalloc(0x800);
    kfree(ptr1);

    char* ptr2 = kmalloc(0x10);
    char* ptr3 = kmalloc(0x10);
    char* ptr4 = kmalloc(0x500);
    char* ptr5 = kmalloc(0x1000);
    kfree(ptr4);
    char* ptr6 = kmalloc(0x2000);
    kfree(ptr2);
    kfree(ptr3);
    kfree(ptr5);
    kfree(ptr6);
}
