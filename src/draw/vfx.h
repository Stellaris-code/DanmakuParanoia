#ifndef VFX_H
#define VFX_H

#include <stdbool.h>

#include "math/vector.h"
#include "color.h"

#define MAX_VFX 256
#define VFX_DATABUF_SIZE 64

typedef bool(draw_callback_t)(void* data, float time);
int vfx_register_impl(int layer, draw_callback_t* callback, void* fx_data, unsigned fx_data_size, const char* fx_name);

#define SPAWN_VFX(layer, name, ...) \
    ({ \
        vfx_##name##_info_t info = (vfx_##name##_info_t){__VA_ARGS__}; \
        int id = vfx_register_impl(layer, &vfx_##name##_draw, (void*)&info, sizeof(info), #name); \
        id; \
    })


typedef struct vfx_sheared_rectangle_info_t
{
    vector2d_t pos;
    color_t col;
    float width, shearing, angle, max_length;
    float effect_duration;
    bool persist;
} vfx_sheared_rectangle_info_t;
bool vfx_sheared_rectangle_draw(void* databuf, float time);

typedef struct vfx_screen_shake_info_t
{
    int x_amplitude;
    int y_amplitude;
    float frequency;
    float duration;

    // internal data
    int last_sample_idx;
    vector2d_t last_sample;
    vector2d_t current_sample;
} vfx_screen_shake_info_t;
bool vfx_screen_shake_draw(void* databuf, float time);

void init_vfx();
void vfx_update(float dt);
void vfx_draw();
void vfx_clear();

#endif // VFX_H
