#include "bullet.h"

#include <assert.h>
#define _GNU_SOURCE
#include <math.h>

#include "draw/draw.h"
#include "draw/drawlist.h"
#include "draw/zorder.h"
#include "gameplay/gamestate.h"

#include "sys/log.h"
#include "sys/window.h"
#include "utils/utils.h"

typedef struct circle_bullet_t
{
    bullet_t bullet;
    circle_info_t info;
} circle_bullet_t;

typedef struct rect_bullet_t
{
    bullet_t bullet;
    rect_info_t info;
} rect_bullet_t;

static circle_bullet_t circle_list[MAX_BULLETS];
static int circle_capacity = 0, circle_count = 0;

static rect_bullet_t rect_list[MAX_BULLETS];
static int rect_capacity = 0, rect_count = 0;

void sincosf(float x, float *sin, float *cos);

#include "bullet_impl_macros.h"

void generic_bullet_update(float dt, bullet_t* bullet)
{
    hitbox_id_t hitbox_id = bullet->hitbox;
    assert(hitbox_id != INVALID_HITBOX_ID);

    update_motion(dt, &bullet->motion);

    vector2d_t pos = absolute_pos(&bullet->motion);

    *bullet->hitbox_center_x = pos.x;
    *bullet->hitbox_center_y = pos.y;
    if (bullet->hitbox_angle)
        *bullet->hitbox_angle = bullet->motion.rotation + 3.1415926f/2; // forgot why I had to rotate the hitbox's angle

    // erase offscreen circle
    if ((pos.x - bullet->visible_radius) >= global_state.game_area_size.x || (pos.y - bullet->visible_radius) >= global_state.game_area_size.y ||
        (pos.x + bullet->visible_radius) < 0 || (pos.y + bullet->visible_radius) < 0)
    {
        //printf("dropped %f %f\n", pos.x, pos.y);
        bullet->to_be_removed = 1;
    }
}

void update_bullets(float dt)
{
    UPDATE_BULLET_TYPE(circle,
                       ({
                           bullet->visible_radius = *GET_HITBOX_FIELD(circle, radius, hitbox_id);
                       }));

    UPDATE_BULLET_TYPE(rect,
                       ({
                           bullet->visible_radius = *GET_HITBOX_FIELD(rect, half_width , hitbox_id) +
                                                    *GET_HITBOX_FIELD(rect, half_height , hitbox_id);
                       }));
}

static sprite_list_entry_t sprite_list[MAX_HITBOX_COUNT];
static int sprite_count;
static texture_t sprite_tex_atlas;

void draw_bullets_callback(void* null)
{
    draw_sprite_batch(sprite_tex_atlas, sprite_list, sprite_count);
}

void draw_bullets()
{
    sprite_count = 0;
    sprite_tex_atlas = (texture_t){0};
    for (int i = 0; i < circle_capacity; ++i)
    {
        hitbox_id_t hitbox_id = circle_list[i].bullet.hitbox;
        if (hitbox_id == INVALID_HITBOX_ID) continue;

        vector2d_t pos = {*circle_list[i].bullet.hitbox_center_x, *circle_list[i].bullet.hitbox_center_y};
        vector2d_t size = {.x = 16, .y = 16};

        sprite_tex_atlas = get_sprite_frame(circle_list[i].bullet.sprite).texture;
        sprite_list[sprite_count].pos = pos;
        sprite_list[sprite_count].size = size;
        sprite_list[sprite_count].angle = 0;
        sprite_list[sprite_count].tint = COL_WHITE;
        sprite_list[sprite_count].spriteframe = get_sprite_frame(circle_list[i].bullet.sprite).spritesheet_rect;
        ++sprite_count;
    }

    for (int i = 0; i < rect_capacity; ++i)
    {
        hitbox_id_t hitbox_id = rect_list[i].bullet.hitbox;
        if (hitbox_id == INVALID_HITBOX_ID) continue;

        float width  = rect_list[i].info.width;
        float height = rect_list[i].info.height;

        vector2d_t pos = {*rect_list[i].bullet.hitbox_center_x, *rect_list[i].bullet.hitbox_center_y};
        vector2d_t size = {width, height};

        sprite_list[sprite_count].pos = pos;
        sprite_list[sprite_count].size = size;
        sprite_list[sprite_count].angle = rect_list[i].bullet.motion.rotation;
        sprite_list[sprite_count].tint = COL_WHITE;
        sprite_list[sprite_count].spriteframe = get_sprite_frame(rect_list[i].bullet.sprite).spritesheet_rect;
        ++sprite_count;
    }

    register_draw_element(NULL, draw_bullets_callback, BULLET_ZORDER);
}

bullet_t* register_bullet(bullet_t bullet_info, void *specific_data)
{
    // if max_* were set to 0, it means that they are to be ignored : set them to +INF
    if (bullet_info.motion.max_rot == 0.0f)
        bullet_info.motion.max_rot = +INFINITY;
    if (bullet_info.motion.max_accel == 0.0f)
        bullet_info.motion.max_accel = +INFINITY;
    if (bullet_info.motion.max_speed == 0.0f)
        bullet_info.motion.max_speed = +INFINITY;
    if (bullet_info.motion.max_angular == 0.0f)
        bullet_info.motion.max_angular = +INFINITY;

    bullet_info.hitbox_angle = NULL;

    bullet_t* ptr = 0;
    if (bullet_info.type == Circle)
    {
        REGISTER_BULLET_TYPE(ptr, circle,
                             ({
                                 *GET_HITBOX_FIELD(circle, radius, hitbox_id) = info->hitbox_radius;
                             }));

    }
    else if (bullet_info.type == Rect)
    {
        REGISTER_BULLET_TYPE(ptr, rect,
                             ({
                                 *GET_HITBOX_FIELD(rect, half_width , hitbox_id) = info->hitbox_width/2.f;
                                 *GET_HITBOX_FIELD(rect, half_height, hitbox_id) = info->hitbox_height/2.f;
                                 bullet_info.hitbox_angle = GET_HITBOX_FIELD(rect, angle, hitbox_id);
                                 *bullet_info.hitbox_angle = bullet_info.motion.rotation;
                             }));
    }

    return ptr;
}

int total_bullet_count()
{
    return circle_count+rect_count;
}

void clear_bullets()
{
    FOR_EACH_BULLET(circle, ({FREE_HITBOX(circle, bullet->hitbox);
                             bullet->hitbox = INVALID_HITBOX_ID;}));
    FOR_EACH_BULLET(rect, ({FREE_HITBOX(rect, bullet->hitbox);
                             bullet->hitbox = INVALID_HITBOX_ID;}));

    rect_count = rect_capacity = 0;
    circle_count = circle_capacity = 0;
}
