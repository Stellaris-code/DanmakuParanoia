#include "bullet.h"

#include <assert.h>
#define _GNU_SOURCE
#include <math.h>

#include "draw/draw.h"
#include "draw/drawlist.h"
#include "draw/zorder.h"
#include "gameplay/gamestate.h"

#include "sys/log.h"
#include "sys/cleanup.h"
#include "sys/window.h"
#include "sys/alloc.h"
#include "utils/utils.h"
#include "utils/resource_pool.h"

#include "thread_support.h"

#include "simd/simd.h"

typedef struct packed_bullet_t
{
    _Alignas(VECTOR_WIDTH) bullet_id_t id[SIMD_ID_PER_REG];

    _Alignas(VECTOR_WIDTH) packed_motion_t motion;

    // internal data
    _Alignas(VECTOR_WIDTH) float  visible_radius[SIMD_FLT_PER_REG]; // NOTE : should it be recomputed for each update?
    _Alignas(VECTOR_WIDTH) unsigned short sprite_frame_id[SIMD_FLT_PER_REG];
} packed_bullet_t;

typedef struct packed_circle_bullet_t
{
    packed_bullet_t b;
    circle_info_t info[SIMD_FLT_PER_REG];
} packed_circle_bullet_t;

typedef struct packed_rect_bullet_t
{
    packed_bullet_t b;

    _Alignas(VECTOR_WIDTH) float width[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float height[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float hitbox_width[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float hitbox_height[SIMD_FLT_PER_REG];
} packed_rect_bullet_t;

typedef struct packed_bullet_pool_t
{
    res_id_t circle_last_id;
    DEFINE_LOCAL_POOL(circle_pool, packed_circle_bullet_t, MAX_BULLETS_PER_TYPE/SIMD_FLT_PER_REG);
    res_id_t rect_last_id;
    DEFINE_LOCAL_POOL(rect_pool, packed_rect_bullet_t, MAX_BULLETS_PER_TYPE/SIMD_FLT_PER_REG);
} packed_bullet_pool_t;

#define MAX_BULLET_POOLS 15

static packed_bullet_pool_t* bullet_pools;
static unsigned bullet_pool_count;
static unsigned current_selected_pool;

void sincosf(float x, float *sin, float *cos);

#include "bullet_impl_macros.h"

// returns true if the bullet pack needs to be freed
static inline bool generic_packed_bullet_update(float dt, packed_bullet_t* packed_bullet,
                                                int area_w, int area_h)
{
    packed_motion_t* motion = &packed_bullet->motion;

    update_motion_simd(dt, motion);

    // TODO : handle root!
    /*
        if (!motion->root)
            pos = (vector2d_t){motion->relative_x, motion->relative_y};
        else
            pos = absolute_pos(motion);
        */

    const simd_vf vec_area_w = vf_set1(area_w);
    const simd_vf vec_area_h = vf_set1(area_h);
    simd_vi id = vi_load(packed_bullet->id);
    simd_vf visible_radius = vf_load(packed_bullet->visible_radius);
    simd_vf x = vf_load(motion->relative_x);
    simd_vf y = vf_load(motion->relative_y);
    simd_vi out_east = vf_cmpgt(x - visible_radius, vec_area_w);
    simd_vi out_west = vf_cmplt(x + visible_radius, vf_set1(0));
    simd_vi out_south = vf_cmpgt(y - visible_radius, vec_area_h);
    simd_vi out_north = vf_cmplt(y + visible_radius, vf_set1(0));
    simd_vi invalid_mask = vi_or(vi_or(out_east, out_west), vi_or(out_south, out_north));
    id = vi_andnot(invalid_mask, id); // mask to zero ids of expired bullets
    vi_store(packed_bullet->id, id);
    return vi_vector_eq(id, vi_set1(0));
}

static int update_bullet_pool(packed_bullet_pool_t* pool, float* pdt)
{
    float dt = *pdt;

    UPDATE_BULLET_TYPE(pool, circle,
                       ({
                       }));


    UPDATE_BULLET_TYPE(pool, rect,
                       ({
                       }));

    return 0;
}


void update_bullets(float dt)
{
/*
    async_tok_t tokens[bullet_pool_count];
    for (unsigned i = 0; i < bullet_pool_count; ++i)
    {
        tokens[i] = async_run_2(update_bullet_pool, &bullet_pools[i], &dt);
    }

    for (unsigned i = 0; i < bullet_pool_count; ++i)
    {
        async_wait(tokens[i]);
    }
*/


#pragma omp parallel for
    for (unsigned i = 0; i < bullet_pool_count; ++i)
    {
        update_bullet_pool(&bullet_pools[i], &dt);
    }

}

// TODO : fast path if bullet pack is full?
void draw_circle_pack(packed_circle_bullet_t* pack)
{
    const vector2d_t circle_size = {.x = 16*2, .y = 16*2};

    packed_bullet_t* packed_bullet = &pack->b;

    /*
    // fast path
    simd_vi ids = vi_load(&packed_bullet->id[0]);
    __m256i vcmp = _mm256_cmpeq_epi32(ids, _mm256_setzero_si256());          // for older SSE use PMOVMSKB
    uint32_t mask = _mm256_movemask_epi8(vcmp);
    if (0 && mask == 0)
    {
        quad_data_t* quad_ptr = prepare_batch_elements(SIMD_FLT_PER_REG);
        for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
        {
            vector2d_t pos = {packed_bullet->motion.relative_x[i], packed_bullet->motion.relative_y[i]};
            quad_ptr[i].sprite_id = packed_bullet->sprite_frame_id[i];
            quad_ptr[i].zorder = 0;
            quad_ptr[i].angle = 0;
            quad_ptr[i].world_pos[0] = pos.x;
            quad_ptr[i].world_pos[1] = pos.y;
            quad_ptr[i].size[0] = circle_size.x;
            quad_ptr[i].size[1] = circle_size.y;
        }
    }
    else
*/
    {
        for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
        {
            if (packed_bullet->id[i] == INVALID_BULLET_ID)
                continue;

            vector2d_t pos = {packed_bullet->motion.relative_x[i], packed_bullet->motion.relative_y[i]};
            draw_sprite_batch_element(circle_size, pos, 0, 0, packed_bullet->sprite_frame_id[i]);
        }
    }
}

void draw_rect_pack(packed_rect_bullet_t* pack)
{
    packed_bullet_t* packed_bullet = &pack->b;
    for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
    {
        if (packed_bullet->id[i] == INVALID_BULLET_ID)
            continue;

        float width  = pack->width[i];
        float height = pack->height[i];

        vector2d_t pos = {packed_bullet->motion.relative_x[i], packed_bullet->motion.relative_y[i]};
        vector2d_t size = {width, height};

        draw_sprite_batch_element(size, pos, packed_bullet->motion.rotation[i], 1, packed_bullet->sprite_frame_id[i]);
    }
}

texture_t sprite_tex_atlas;

static int draw_bullet_pool(packed_bullet_pool_t* pool)
{
    FOR_EACH_PACKED_BULLET(pool, circle,
                           ({
                                   draw_circle_pack(circle_entry);
                           }));
    FOR_EACH_PACKED_BULLET(pool, rect,
                           ({
                                   draw_rect_pack(rect_entry);
                           }));
}

// FIXME : remplacer par le vrai dt
void draw_bullets_callback(void* null)
{
    (void)null;

    begin_draw_sprite_batch(get_spritesheet_texture(global_state.sprite_atlas));
    for (unsigned i = 0; i < bullet_pool_count; ++i)
    {
        draw_bullet_pool(&bullet_pools[i]);
    }
    end_draw_sprite_batch();
}

void draw_bullets()
{
    register_draw_element(NULL, draw_bullets_callback, BULLET_ZORDER);
}

// returns a bullet id local to this bullet pool
static inline uint32_t alloc_bullet(res_id_t* last_id, res_pool_t* pool)
{
    packed_circle_bullet_t* ptr;
    // Search in the last allocated block
    if (*last_id != INVALID_RES_ID && res_pool_used(pool, *last_id))
    {
        ptr = res_pool_get(pool, *last_id);
        for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
        {
            if (ptr->b.id[i] != INVALID_BULLET_ID)
                continue;

            // available subentry
            return (*last_id*SIMD_FLT_PER_REG + i) + 1;
        }
    }

    res_id_t packed_id = res_pool_alloc(pool);
    if (packed_id == INVALID_RES_ID)
        return INVALID_BULLET_ID;

    *last_id = packed_id;
    ptr = res_pool_get(pool, packed_id);
    for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
        ptr->b.id[i] = INVALID_BULLET_ID;
    return (*last_id*SIMD_FLT_PER_REG) + 1;
}

bullet_id_t register_bullet(bullet_info_t bullet_info, motion_data_t motion, void *specific_data)
{
    // if max_* were set to 0, it means that they are to be ignored : set them to +INF
    if (motion.max_rot == 0.0f)
        motion.max_rot = +INFINITY;
    if (motion.max_accel == 0.0f)
        motion.max_accel = +INFINITY;
    if (motion.max_speed == 0.0f)
        motion.max_speed = +INFINITY;
    if (motion.max_angular == 0.0f)
        motion.max_angular = +INFINITY;

    packed_bullet_t* ptr = 0;

    unsigned selected_pool_id = current_selected_pool;
    packed_bullet_pool_t* selected_pool = &bullet_pools[selected_pool_id];

    unsigned i;
    bullet_id_t local_bullet_id;
    if (bullet_info.type == Circle)
    {
        local_bullet_id = alloc_bullet(&selected_pool->circle_last_id, &selected_pool->circle_pool);
        if (local_bullet_id == INVALID_BULLET_ID)
        {
            trace_log(LOG_WARNING, "Out of " "circle" " bullets");
            return INVALID_BULLET_ID;
        }
        i = (local_bullet_id-1)%SIMD_FLT_PER_REG;

        packed_circle_bullet_t* entry_ptr = res_pool_get(&selected_pool->circle_pool, (local_bullet_id-1)/SIMD_FLT_PER_REG);
        assert(entry_ptr);
        ptr = &entry_ptr->b;

        circle_info_t* info = specific_data;
        ptr->visible_radius[i] = info->hitbox_radius;
        entry_ptr->info[i] = *info;
    }
    else if (bullet_info.type == Rect)
    {
        local_bullet_id = alloc_bullet(&selected_pool->rect_last_id, &selected_pool->rect_pool);
        if (local_bullet_id == INVALID_BULLET_ID)
        {
            trace_log(LOG_WARNING, "Out of " "rect" " bullets");
            return INVALID_BULLET_ID;
        }
        i = (local_bullet_id-1)%SIMD_FLT_PER_REG;

        packed_rect_bullet_t* entry_ptr = res_pool_get(&selected_pool->rect_pool, (local_bullet_id-1)/SIMD_FLT_PER_REG);
        assert(entry_ptr);
        ptr = &entry_ptr->b;

        rect_info_t* info = specific_data;
        // normalize the bullet angle
        ptr->motion.rotation[i] -= floorf(motion.rotation/(2*M_PI))*(2*M_PI);
        ptr->visible_radius[i] = (info->hitbox_width/2.f +
                                  info->hitbox_height/2.f)*2.f; // x2 margin, assuming no bullet has too small of a hitbox compared to its sprite

        entry_ptr->width[i] = info->width;
        entry_ptr->height[i] = info->height;
        entry_ptr->hitbox_width[i] = info->hitbox_width;
        entry_ptr->hitbox_height[i] = info->hitbox_height;
    }
    else
    {
        assert("invalid bullet type" && false);
        abort();
    }

    motion_simd_load(&ptr->motion, i, &motion);
    ptr->id[i] = local_bullet_id;
    ptr->sprite_frame_id[i] = bullet_info.sprite;

    assert(local_bullet_id != INVALID_BULLET_ID);

    assert(selected_pool_id <= MAX_BULLET_POOLS);
    bullet_id_t global_bullet_id = local_bullet_id;
    global_bullet_id <<= 4; // leave some space for encoding the bullet pool it belongs to
    global_bullet_id |= selected_pool_id&0b1111;

    ++current_selected_pool;
    if (current_selected_pool >= bullet_pool_count)
        current_selected_pool = 0;

    return global_bullet_id;
}

// FIXME : not an exact count
int total_bullet_count()
{
    int count = 0;
    for (unsigned i = 0; i < bullet_pool_count; ++i)
    {
        packed_bullet_pool_t* pool = &bullet_pools[i];
        count += pool->circle_pool.count*SIMD_FLT_PER_REG + pool->rect_pool.count*SIMD_FLT_PER_REG;
    }
    return count;
}

void clear_bullets()
{
    for (unsigned i = 0; i < bullet_pool_count; ++i)
    {
        packed_bullet_pool_t* pool = &bullet_pools[i];
        res_pool_clear(&pool->circle_pool);
        res_pool_clear(&pool->rect_pool);
    }
    bullet_pool_count = 0;
}

void cleanup_bullet_manager()
{
    clear_bullets();
    if (bullet_pools)
        danpa_free(bullet_pools);
    bullet_pools = NULL;
}

void init_bullet_manager()
{
    assert(bullet_pools == NULL);
    // create as many bullet pools as there are available hardware threads (up to MAX_BULLET_POOLS)
    bullet_pool_count = MIN(thrd_hardware_threads(), MAX_BULLET_POOLS);
    bullet_pools = danpa_calloc(bullet_pool_count, sizeof(packed_bullet_pool_t));
    for (unsigned i = 0; i < bullet_pool_count; ++i)
    {
        packed_bullet_pool_t* pool = &bullet_pools[i];
        pool->circle_last_id = INVALID_RES_ID;
        pool->rect_last_id = INVALID_RES_ID;
        INIT_LOCAL_POOL(pool->, circle_pool);
        INIT_LOCAL_POOL(pool->, rect_pool);
    }

    register_cleanup(cleanup_bullet_manager, GamestateEnd);
}
