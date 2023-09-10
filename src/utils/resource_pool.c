#include "resource_pool.h"

#include "sys/log.h"

// Returns the first used block before this one, or INVALID_RES_ID if none exist
static res_id_t res_get_first_used_left(res_pool_t* pool, res_id_t start_id)
{
    pool_entry_meta_t* meta = res_pool_get_meta(pool, start_id);
    assert(meta);
    assert(meta->used);

    res_id_t target = start_id;
    do
    {
        if (target == 0)
        {
            target = INVALID_RES_ID;
            break;
        }
        --target;
        meta = res_pool_get_meta(pool, target);
    } while (meta && !meta->used);
    //printf("first used : %d\n", target);
    return target;
}

res_id_t res_pool_alloc(res_pool_t *pool)
{
    // Pool is full
    uint32_t free_blocks = pool->max_count - pool->count;
    if (free_blocks == 0)
    {
        trace_log(LOG_INFO, "'%s' : Could not allocate an ID, resource pool is full\n", pool->name);
        return INVALID_RES_ID;
    }

    if (pool->num_initialized < pool->max_count)
    {
        pool_entry_meta_t* new_init = (pool_entry_meta_t*)((uint8_t*)pool->entries + pool->num_initialized*pool->elem_size);
        new_init->next_free = pool->num_initialized + 1;
        new_init->used = false;
        ++pool->num_initialized;
    }

    ++pool->count;
    pool_entry_meta_t* meta = res_pool_get_meta(pool, pool->free_ptr);
    assert(meta != NULL);
    assert(!meta->used);

    meta->used = true;
    res_id_t alloc_id = pool->free_ptr;
    pool->free_ptr = meta->next_free;

    res_id_t prev_used = res_get_first_used_left(pool, alloc_id);
    // Insert inside the used block list
    if (prev_used != INVALID_RES_ID)
    {
        pool_entry_meta_t* prev_used_meta = res_pool_get_meta(pool, prev_used);
        assert(prev_used_meta);
        assert(prev_used_meta->used);
        if (prev_used_meta->next != INVALID_RES_ID)
        {
            pool_entry_meta_t* next_used_meta = res_pool_get_meta(pool, prev_used_meta->next);
            assert(next_used_meta);
            assert(next_used_meta->used);
            next_used_meta->prev = alloc_id;
        }
        meta->prev = prev_used;
        meta->next = prev_used_meta->next;
        prev_used_meta->next = alloc_id;
        //printf("Updating prev with %d\n", alloc_id);
    }
    // Head of the used block list
    else
    {
        if (pool->first_used != INVALID_RES_ID)
        {
            pool_entry_meta_t* next_used_meta = res_pool_get_meta(pool, pool->first_used);
            assert(next_used_meta);
            assert(next_used_meta->used);
            next_used_meta->prev = alloc_id;
        }

        meta->prev = INVALID_RES_ID;
        meta->next = pool->first_used;
        pool->first_used = alloc_id;
    }

    //printf("Allocd %d, prev %d next %d\n", alloc_id, meta->prev, meta->next);

    if (meta->prev != INVALID_RES_ID)
        assert(meta->prev < alloc_id);
    if (meta->next != INVALID_RES_ID)
        assert(meta->next > alloc_id);

    return alloc_id;
}

void res_pool_free(res_pool_t* pool, res_id_t id)
{
    pool_entry_meta_t* meta = res_pool_get_meta(pool, id);
    assert(meta);
    if (meta == NULL) // please clang-tidy
        return;
    assert(meta->used);

    // Update the used block linked list
    //printf("Freeing %d, prev %d next %d, next free %d\n", id, meta->prev, meta->next, pool->free_ptr);
    pool_entry_meta_t* prev = res_pool_get_meta(pool, meta->prev);
    if (prev)
    {
        assert(prev->used);
        prev->next = meta->next;
    }
    pool_entry_meta_t* next = res_pool_get_meta(pool, meta->next);
    if (next)
    {
        assert(next->used);
        next->prev = meta->prev;
    }

    if (pool->first_used == id)
        pool->first_used = meta->next;

    meta->used = false;
    meta->next_free = pool->free_ptr;
    pool->free_ptr = id;

    assert(pool->count != 0);
    --pool->count;

    //printf("deleted %d\n", id);
}

void res_pool_clear(res_pool_t *pool)
{
    pool->num_initialized = 0;
    pool->count = 0;
    pool->first_used = INVALID_RES_ID;
    pool->free_ptr = 0;
}
