#include "vfx.h"

//#include <rlgl.h>

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "utils/utils.h"
#include "sys/log.h"
#include "sys/cleanup.h"

#include "draw/drawlist.h"

typedef bool(draw_callback_t)(void* data, float time);

typedef struct vfx_entry_t
{
    uint8_t databuf[VFX_DATABUF_SIZE]; // 64 bytes of data to store the effect's state
    float time_acc;
    int layer;
    draw_callback_t* draw; // returns true if it can be destroyed
} vfx_entry_t;

static vfx_entry_t vfx_entries[MAX_VFX];

void vfx_update(float dt)
{
    for (size_t i = 0; i < MAX_VFX; ++i)
    {
        if (vfx_entries[i].draw)
            vfx_entries[i].time_acc += dt;
    }
}

static void vfx_draw_callback(void* vfx_entry_ptr)
{
    vfx_entry_t* entry = vfx_entry_ptr;

    bool delete = entry->draw(&entry->databuf, entry->time_acc);
    if (delete)
        entry->draw = NULL; // delete the entry
}

void vfx_draw()
{
    for (size_t i = 0; i < MAX_VFX; ++i)
    {
        if (vfx_entries[i].draw)
        {
            register_draw_element(&vfx_entries[i], vfx_draw_callback, vfx_entries[i].layer);
        }
    }
}

int vfx_register_impl(int layer, draw_callback_t* callback, void* fx_data, unsigned fx_data_size, const char* fx_name)
{
    assert(fx_data_size <= VFX_DATABUF_SIZE);

    for (size_t i = 0; i < MAX_VFX; ++i)
    {
        if (!vfx_entries[i].draw) // unused
        {
            memcpy(&vfx_entries[i].databuf, fx_data, fx_data_size);
            vfx_entries[i].time_acc = 0.0f;
            vfx_entries[i].draw = callback;
            vfx_entries[i].layer = layer;
            return i;
        }
    }

    // VFX entries full
    trace_log(LOG_WARNING, "Could not allocate VFX '%s'", fx_name);
    return -1;
}
#define VFX_REGISTER(callback, data) \
    vfx_register_impl(callback, &data, sizeof(data), #callback)

// all VFX here

void vfx_clear()
{
    for (size_t i = 0; i < MAX_VFX; ++i)
    {
        vfx_entries[i].draw = NULL;
    }
}

void init_vfx()
{
    register_cleanup(vfx_clear);
}
