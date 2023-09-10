#ifndef RESOURCE_POOL_H
#define RESOURCE_POOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

// Implementation based on
// Kenwright, B. (2012). Fast efficient fixed-size memory pool: No loops and no overhead. In The Third International Conference on Computational Logics, Algebras, Programming, Tools, and Benchmarking.

typedef uint32_t res_id_t;

#define INVALID_RES_ID ((res_id_t)-1)

typedef struct pool_entry_meta_t
{
    uint8_t used; // 0 if free, 1 if used
    union
    {
        // Free
        struct
        {
            res_id_t next_free; // next free block, if the block is free
        };
        // Used
        struct
        {
            res_id_t prev;
            res_id_t next;
        };
    };
} pool_entry_meta_t;

typedef struct res_pool_t
{
    const char* name;
    uint32_t count;
    uint32_t max_count;
    uint32_t elem_size;
    uint32_t data_offset; // offset to the user data inside the anonymous entry struct
    res_id_t first_used; // pointer to the first used block of memory
    res_id_t free_ptr; // pointer to the head of the free list
    uint32_t num_initialized;
    void* entries; // non-owning pointer to the memory area containing the pool objects
} res_pool_t;

#define DEFINE_RESOURCE_POOL(name, type, size) \
    struct {\
        pool_entry_meta_t meta; \
        type data; \
        } name##_pool_data[size]; \
    res_pool_t name = {#name, 0, size, sizeof(name##_pool_data[0]), offsetof(typeof(name##_pool_data[0]), data), INVALID_RES_ID, 0, 0, &name##_pool_data}; \

#define DEFINE_LOCAL_POOL(name, type, size) \
    struct {\
        pool_entry_meta_t meta; \
        type data; \
        } name##_pool_data[size]; \
    res_pool_t name;

#define INIT_LOCAL_POOL(accessor, name) \
    accessor name = (res_pool_t){#name, 0, sizeof(accessor name##_pool_data)/sizeof(accessor name##_pool_data[0]), sizeof(accessor name##_pool_data[0]), offsetof(typeof(accessor name##_pool_data[0]), data), INVALID_RES_ID, 0, 0, &accessor name##_pool_data};

#define FOR_EACH_RES(pool, id, type, entry_name) \
    FOR_EACH_RES_ACCESSOR(, pool, id, type, entry_name)

#define FOR_EACH_RES_ACCESSOR(accessor, pool, id, type, entry_name) \
    type entry_name; \
    for (res_id_t id = res_pool_first(&accessor pool), id_next = id != INVALID_RES_ID ? accessor pool##_pool_data[id].meta.next : INVALID_RES_ID; \
 id != INVALID_RES_ID && (entry_name = &accessor pool##_pool_data[id].data, id_next = accessor pool##_pool_data[id].meta.next); \
    id = id_next) \

// ...not O(1) but, uh, close!
res_id_t res_pool_alloc(res_pool_t* pool);
// O(1)
void res_pool_free(res_pool_t* pool, res_id_t id);


// O(1)
static inline pool_entry_meta_t *res_pool_get_meta(res_pool_t *pool, res_id_t id)
{
    if (id == INVALID_RES_ID)
        return NULL;
    assert(id < pool->num_initialized);

    return (pool_entry_meta_t*)((uint8_t*)pool->entries + id*pool->elem_size);
}

// O(1)
static inline bool res_pool_used(res_pool_t *pool, res_id_t id)
{
    pool_entry_meta_t* meta = res_pool_get_meta(pool, id);
    return meta && meta->used;
}

// O(1)
static inline res_id_t res_pool_next(res_pool_t* pool, res_id_t id)
{
    pool_entry_meta_t* meta = res_pool_get_meta(pool, id);
    if (!meta || !meta->used)
        return INVALID_RES_ID;

    return meta->next;
}
// O(1)
static inline void* res_pool_get(res_pool_t* pool, res_id_t id)
{
    pool_entry_meta_t* meta_base = res_pool_get_meta(pool, id);
    if (!meta_base)
        return NULL;
    assert(meta_base->used);

    return (uint8_t*)meta_base + pool->data_offset;
}
// O(1)
static inline res_id_t res_pool_first(res_pool_t* pool)
{
    return pool->first_used;
}
// O(1)
void res_pool_clear(res_pool_t* pool);

#endif // RESOURCE_POOL_H
