#define FOR_EACH_PACKED_BULLET(global_pool, type, action) \
    FOR_EACH_RES((global_pool)->type##_pool, packed_bullet_id, packed_##type##_bullet_t*, type##_entry) \
    { \
    packed_bullet_t* packed_bullet = &type##_entry->b; \
    assert(packed_bullet_id != INVALID_RES_ID); \
    action; \
    }

#define UPDATE_BULLET_TYPE(global_pool, type, specific_action) \
    FOR_EACH_PACKED_BULLET(global_pool, type, ({ \
        specific_action; \
        bool to_be_freed = generic_packed_bullet_update(dt, packed_bullet, global_state.game_area_size.x, global_state.game_area_size.y); \
        if (to_be_freed) \
            res_pool_free(&(global_pool)->type##_pool, packed_bullet_id); \
    }))
