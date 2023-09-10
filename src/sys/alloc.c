#include "alloc.h"

#define USE_CUSTOM_ALLOCATORS

#ifdef USE_CUSTOM_ALLOCATORS

#define DANPA_ALLOC_MAGIC 0xCAFEB055

#include "sys/log.h"

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

static size_t total_allocated_mem;
unsigned danpa_alloc_count;

typedef struct mem_block_t
{
    union
    {
        struct
        {
            uint32_t magic;
            size_t sz;
        };
        max_align_t align; // To conserve the alignment of the following data
    };
    unsigned char data[0];
} mem_block_t;

void* danpa_alloc(size_t size)
{
    mem_block_t* block = malloc(size + sizeof(mem_block_t));
    void* ptr = &block->data;
    if (ptr)
        total_allocated_mem += size + sizeof(mem_block_t);
    else
    {
        trace_log(LOG_FATAL, "Unable to allocate %lld bytes of memory\n", size);
        abort();
    }

    ++danpa_alloc_count;
    block->sz = size;
    block->magic = DANPA_ALLOC_MAGIC;
    return ptr;
}

void* danpa_realloc(void* ptr, size_t size)
{
    if (ptr == NULL)
        return danpa_alloc(size);

    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    assert(block->magic == DANPA_ALLOC_MAGIC);
    total_allocated_mem -= block->sz;

    block = realloc(block, size + sizeof(mem_block_t));
    void* new_ptr = &block->data;
    if (new_ptr)
        total_allocated_mem += size;
    else
    {
        trace_log(LOG_FATAL, "Unable to allocate %lld bytes of memory\n", size);
        abort();
    }

    ++danpa_alloc_count;

    block->sz = size;
    return new_ptr;
}

void* danpa_calloc(size_t n, size_t size)
{
    mem_block_t* block = calloc(1, n*size + sizeof(mem_block_t));
    void* ptr = &block->data;
    if (ptr)
        total_allocated_mem += n*size + sizeof(mem_block_t);
    else
    {
        trace_log(LOG_FATAL, "Unable to allocate %lld bytes of memory\n", n*size);
        abort();
    }

    ++danpa_alloc_count;
    block->sz = n*size;
    block->magic = DANPA_ALLOC_MAGIC;
    return ptr;
}

void danpa_free(void* ptr)
{
    if (ptr == NULL)
        return;

    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    assert(block->magic == DANPA_ALLOC_MAGIC);
    total_allocated_mem -= block->sz + sizeof(mem_block_t);
    free(block);
}
unsigned danpa_allocated_mem()
{
    return total_allocated_mem;
}

#else

void* danpa_alloc(size_t size)
{
    return malloc(size);
}

void* danpa_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void* danpa_calloc(size_t n, size_t size)
{
    return calloc(n, size);
}

void danpa_free(void* ptr)
{
    free(ptr);
}
unsigned danpa_allocated_mem()
{
    return -1;
}

#endif

