#include "resources/sfx_handler.h"

#include <raudio.h>

#include "sys/log.h"
#include "sys/cleanup.h"

DEFINE_RESOURCE_POOL(sfx_pool, Sound, MAX_SFX_ENTRIES);

static void cleanup_sfx()
{
    StopSoundMulti();
    FOR_EACH_RES(sfx_pool, id, Sound*, ptr)
    {
        StopSound(*ptr);
        UnloadSound(*ptr);
    }
    res_pool_clear(&sfx_pool);
}

void init_sfx_handler()
{
    register_cleanup(cleanup_sfx, GamestateEnd);
}

sfx_id_t load_sfx(const char* path)
{
    sfx_id_t id = res_pool_alloc(&sfx_pool);
    if (id == INVALID_RES_ID)
        return id;

    Sound* ptr = res_pool_get(&sfx_pool, id);
    *ptr = LoadSound(path);
    if (ptr->sampleCount == 0)
    {
        trace_log(LOG_WARNING, "Error loading sound file '%s'\n", path);
        return INVALID_RES_ID;
    }

    return id;
}

void play_sfx(sfx_id_t id)
{
    Sound* ptr = res_pool_get(&sfx_pool, id);
    if (!ptr)
        return;

    PlaySoundMulti(*ptr);
}

void set_sfx_volume(sfx_id_t id, float volume)
{
    Sound* ptr = res_pool_get(&sfx_pool, id);
    if (!ptr)
        return;

    SetSoundVolume(*ptr, volume);
}
