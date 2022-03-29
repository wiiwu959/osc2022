#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stddef.h>

#define MEM_HEAD (char*)0x9000000
#define MEM_TAIL (char*)0x9ffffff

void* simple_malloc(size_t size);

#endif  /*_ALLOCATOR_H */