#include <allocator.h>
#include <stddef.h>
#include <list.h>
#include <printf.h>
#include <mini_uart.h>

#define DEBUG

char* malloc_ptr = MEM_HEAD;

struct list_head frame_freelist[PAGE_MAX_FREE];
page_head frame_array[PAGE_TOTAL];

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

int addr2index(void* ptr) {
    return ((char*)(ptr) - (char*)PAGE_BASE) / PAGE_SIZE;
}

void* index2addr(int num) {
    return (void*)(PAGE_BASE + num * PAGE_SIZE);
}

void page_init()
{
    // init free list
    for (int i = 0; i < PAGE_MAX_FREE; i++) {
        INIT_LIST_HEAD(&frame_freelist[i]);
    }

    int max_size = 1 << PAGE_MAX_FREE;
    for (int i = 0; i < PAGE_TOTAL; i += max_size) {
        struct list_head * added;
        frame_array[i].val = PAGE_MAX_FREE - 1;
        frame_array[i].status = FREE_PAGE;
        added = (struct list_head *)index2addr(i);
        list_add_tail(added, &frame_freelist[PAGE_MAX_FREE - 1]);
    }
    chunk_init();
}

void *alloc_page(int num)
{
    int target_exp = num - 1 ? sizeof(num - 1) * 8 - __builtin_clz(num - 1) : 0;

    for (int available = target_exp; available < PAGE_MAX_FREE; available++) {
        if (!list_empty(&frame_freelist[available])) {
            struct list_head* cur = frame_freelist[available].next;
            list_del(cur);

            for (int i = available; i > target_exp; i--) {
                struct list_head* add = (char*)cur + (PAGE_SIZE * (1 << (i - 1)));
                list_add(add, &frame_freelist[i - 1]);
                frame_array[addr2index(add)].val = i - 1;
                frame_array[addr2index(add)].status = FREE_PAGE;
#ifdef DEBUG
                printf("[*] Split exp %d frame to exp %d frame. \n", i, i - 1);
#endif
            }

            int index = addr2index(cur);
            frame_array[index].val = target_exp;
            frame_array[index].status = ALLOCATED;
#ifdef DEBUG
            printf("[*] Allocate %d pages using exp %d. \n", num, target_exp);
#endif
            return cur;
        }
    }
    return NULL;
}

void free_page(struct list_head* target) {
    int index = addr2index(target);

#ifdef DEBUG
    printf("[*] index = %d. \n", index);
#endif

    if (frame_array[index].status == FREE_PAGE) {
        printf("[*] Double free.");
        return;
    }

    int exp = frame_array[index].val;
    list_add(target, &frame_freelist[exp]);
    int buddy_index = index ^ (1 << exp);

    while (exp + 1 < PAGE_MAX_FREE &&
        frame_array[buddy_index].status == FREE_PAGE &&
        frame_array[buddy_index].val == exp) {
        
        struct list_head* buddy = (struct list_head*)index2addr(buddy_index);
        list_del(buddy);

#ifdef DEBUG
        printf("[*] Merge index %d with %d, back to free list exp %d.\n", index, buddy_index, exp + 1);
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


void page_test(void)
{
    char* ptr1 = alloc_page(1);
    free_page(ptr1);
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
        frame_array[frame_index].val = chunk_size[index];

        int page_size = PAGE_SIZE;
        while (page_size - size >= 0) {
            list_add_tail((struct list_head*)new_page, &chunk_freelist[index]);
            new_page = (char*)new_page + size;
            page_size -= size;
        }
    }
#ifdef DEBUG
    printf("[*] Allocate chunk size: %d\n", chunk_size[index]);
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
        if (size & 0xFFF > 0) { page_num++; }
        return alloc_page(page_num);
    }
}

void kfree(void *ptr)
{
    int index = ((char*)((int)ptr & (~0xFFF)) - (char*)PAGE_BASE) / PAGE_SIZE;
    if(frame_array[index].status == IS_CHUNK) {
        int chunk_index = frame_array[index].val;
        list_add_tail((struct list_head*)ptr, &chunk_freelist[chunk_index]);
    } else {
        free_page(ptr);
    }
}

void mem_test() {
    char* ptr1 = kmalloc(0x800);
    kfree(ptr1);
}