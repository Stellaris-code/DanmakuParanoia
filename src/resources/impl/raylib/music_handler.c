#include "resources/music_handler.h"

#include <assert.h>

#include <raylib.h>

#include "sys/log.h"
#include "sys/cleanup.h"

typedef struct music_t
{
    bool loaded;
    Music rl_music;
} music_t;

static music_t music_entries[MAX_MUSIC_ENTRIES];

music_id_t current_bgm = INVALID_MUSIC_ID;

music_id_t load_music(const char *path)
{
    Music rl_music = LoadMusicStream(path);
    if (rl_music.stream.buffer == 0)
    {
        trace_log(LOG_WARNING, "Could not load music '%s'\n", path);
        return INVALID_MUSIC_ID;
    }

    for (int i = 0; i < MAX_MUSIC_ENTRIES; ++i)
    {
        if (!music_entries[i].loaded) // free entry
        {
            music_entries[i].loaded = true;
            music_entries[i].rl_music = rl_music;
            return i;
        }
    }

    trace_log(LOG_WARNING, "Too many musics are loaded, couldn't load '%s'\n", path);
    return INVALID_MUSIC_ID;
}

void play_music(music_id_t music)
{
    music_t entry = music_entries[music];
    assert(entry.loaded);

    PlayMusicStream(entry.rl_music);
}

void resume_music(music_id_t music)
{
    music_t entry = music_entries[music];
    assert(entry.loaded);

    ResumeMusicStream(entry.rl_music);
}

void pause_music(music_id_t music)
{
    music_t entry = music_entries[music];
    assert(entry.loaded);

    PauseMusicStream(entry.rl_music);
}

void update_music_streams()
{
    for (int i = 0; i < MAX_MUSIC_ENTRIES; ++i)
    {
        if (music_entries[i].loaded)
        {
            UpdateMusicStream(music_entries[i].rl_music);
        }
    }
}

void set_music_volume(music_id_t music, float volume)
{
    music_t entry = music_entries[music];
    assert(entry.loaded);

    SetMusicVolume(entry.rl_music, volume);
}

void stop_music(music_id_t music)
{
    music_t entry = music_entries[music];
    assert(entry.loaded);

    StopMusicStream(entry.rl_music);
}

void cleanup_music_handler()
{
    for (int i = 0; i < MAX_MUSIC_ENTRIES; ++i)
    {
        if (music_entries[i].loaded)
        {
            UnloadMusicStream(music_entries[i].rl_music);
        }
    }
}

void init_music_handler()
{
    register_cleanup(cleanup_music_handler);
}
