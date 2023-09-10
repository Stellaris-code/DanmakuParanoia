#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

void* danpa_alloc(size_t size);
void* danpa_realloc(void* ptr, size_t size);
void* danpa_calloc(size_t n, size_t size);
void  danpa_free(void* ptr);

// Returns the size of the memory block
size_t danpa_mem_size(void* ptr);
unsigned danpa_allocated_mem();

extern unsigned danpa_alloc_count;

#endif // ALLOC_H
