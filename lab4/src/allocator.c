#include <allocator.h>
#include <stddef.h>
#include <list.h>
#include <printf.h>
#include <mini_uart.h>

#define DEBUG

char* malloc_ptr = MEM_HEAD;
struct list_head frame_freelist[PAGE_MAX_FREE];
page_head frame_array[PAGE_TOTAL];

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
        added = (struct list_head *)(PAGE_BASE + i * PAGE_SIZE);
        list_add_tail(added, &frame_freelist[PAGE_MAX_FREE - 1]);
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
                struct list_head* add = (char*)cur + (PAGE_SIZE * (1 << (i - 1)));
                list_add(add, &frame_freelist[i - 1]);
                frame_array[((int)add - PAGE_BASE) / PAGE_SIZE].val = i - 1;
                frame_array[((int)add - PAGE_BASE) / PAGE_SIZE].status = FREE_PAGE;
#ifdef DEBUG
                printf("[*] Split exp %d frame to exp %d frame. \n", i, i - 1);
#endif
            }

            int index = ((char*)(cur) - (char*)PAGE_BASE) / PAGE_SIZE;
            frame_array[index].val = target_exp;
            frame_array[index].status = ALLOCATED;
#ifdef DEBUG
            printf("[*] Allocate %d pages using exp %d. \n", num, target_exp);
            uart_send_hex((unsigned int)cur);
            printf("\n");
#endif
            return cur;
        }
    }
    return NULL;
}

void free_page(struct list_head* target) {
    int index = ((char*)(target) - (char*)PAGE_BASE) / PAGE_SIZE;

#ifdef DEBUG
    printf("[*] index = %d. \n", index);
#endif

    if (frame_array[index].status != ALLOCATED) {
        printf("[*] Double free.");
        return;
    }

    int exp = frame_array[index].val;
    list_add(target, &frame_freelist[exp]);
    int buddy_index = index ^ (1 << exp);

    while (exp + 1 < PAGE_MAX_FREE &&
        frame_array[buddy_index].status == FREE_PAGE &&
        frame_array[buddy_index].val == exp) {
        
        struct list_head* buddy = (struct list_head*)(PAGE_BASE + (buddy_index * PAGE_SIZE));
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


void page_test(void) {
    char* ptr1 = alloc_page(1);
    free_page(ptr1);
}