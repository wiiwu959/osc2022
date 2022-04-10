#include <allocator.h>
#include <stddef.h>

char* malloc_ptr = MEM_HEAD;

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