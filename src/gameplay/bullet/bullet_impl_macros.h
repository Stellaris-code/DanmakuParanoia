#define FOR_EACH_BULLET(type, action) \
    for (int i = 0; i < type##_capacity; ++i) \
    { \
    bullet_t* bullet = &type##_list[i].bullet; \
    hitbox_id_t hitbox_id = bullet->hitbox; \
    if (hitbox_id == INVALID_HITBOX_ID) continue; \
    action; \
    }

#define UPDATE_BULLET_TYPE(type, specific_action) \
    FOR_EACH_BULLET(type, ({ \
        specific_action; \
        generic_bullet_update(dt, bullet); \
        if (bullet->to_be_removed) \
        { \
            FREE_HITBOX(type, bullet->hitbox); \
            bullet->hitbox = INVALID_HITBOX_ID; /* free the bullet_list entry */ \
            --type##_count; \
        } \
    }))


#define REGISTER_BULLET_TYPE(ptr, type, specific_action) \
    do { \
            if (type##_capacity >= MAX_BULLETS) \
    { \
        trace_log(LOG_WARNING, "Out of " #type " bullets"); \
        return 0; \
    } \
    type##_info_t* info = specific_data; \
    type##_list[type##_capacity].info = *info; \
    hitbox_id_t hitbox_id = REGISTER_HITBOX(type, type##_capacity+1); \
    if (hitbox_id == INVALID_HITBOX_ID) \
    { \
        trace_log(LOG_WARNING, "Out of hitboxes"); \
        return 0; \
    } \
    bullet_info.hitbox = hitbox_id; \
    bullet_info.to_be_removed = false; \
    bullet_info.hitbox_center_x = GET_HITBOX_FIELD(type, center_x, hitbox_id); \
    bullet_info.hitbox_center_y = GET_HITBOX_FIELD(type, center_y, hitbox_id); \
    vector2d_t pos = absolute_pos(&bullet_info.motion); \
    *bullet_info.hitbox_center_x = pos.x; \
    *bullet_info.hitbox_center_y = pos.y; \
    specific_action; \
    type##_list[type##_capacity].bullet = bullet_info; \
    ptr = &type##_list[type##_capacity].bullet; \
    ++type##_capacity; \
    ++type##_count; \
    } while (0)
