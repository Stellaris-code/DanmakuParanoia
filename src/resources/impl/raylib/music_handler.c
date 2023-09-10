#include "resources/music_handler.h"

#include <assert.h>

#include <raylib.h>

#include "sys/log.h"
#include "sys/cleanup.h"

typedef struct music_t
{
    Music rl_music;
} music_t;

static music_t music_entries[MAX_MUSIC_ENTRIES];
static uint32_t music_count = 0;

music_id_t current_bgm = INVALID_MUSIC_ID;

music_id_t load_music(const char *path)
{
    if (music_count >= MAX_MUSIC_ENTRIES)
    {
        trace_log(LOG_WARNING, "Too many musics are loaded, couldn't load '%s'\n", path);
        return INVALID_MUSIC_ID;
    }

    Music rl_music = LoadMusicStream(path);
    if (rl_music.stream.buffer == 0)
    {
        trace_log(LOG_WARNING, "Could not load music '%s'\n", path);
        return INVALID_MUSIC_ID;
    }

    uint32_t id = music_count;
    music_entries[id].rl_music = rl_music;
    ++music_count;

    return id;
}

void play_music(music_id_t music)
{
    assert(music < music_count);
    music_t entry = music_entries[music];

    PlayMusicStream(entry.rl_music);
}

void resume_music(music_id_t music)
{
    assert(music < music_count);
    music_t entry = music_entries[music];

    ResumeMusicStream(entry.rl_music);
}

void pause_music(music_id_t music)
{
    assert(music < music_count);
    music_t entry = music_entries[music];

    PauseMusicStream(entry.rl_music);
}

void update_music_streams()
{
    for (uint32_t i = 0; i < music_count; ++i)
    {
            UpdateMusicStream(music_entries[i].rl_music);
    }
}

void set_music_volume(music_id_t music, float volume)
{
    assert(music < music_count);
    music_t entry = music_entries[music];

    SetMusicVolume(entry.rl_music, volume);
}

void stop_music(music_id_t music)
{
    assert(music < music_count);
    music_t entry = music_entries[music];

    StopMusicStream(entry.rl_music);
}

void cleanup_music_handler()
{
    for (uint32_t i = 0; i < music_count; ++i)
    {
        UnloadMusicStream(music_entries[i].rl_music);
    }
    music_count = 0;
}

void init_music_handler()
{
    register_cleanup(cleanup_music_handler, GamestateEnd);
}
