#ifndef MUSIC_HANDLER_H
#define MUSIC_HANDLER_H

#include <stdint.h>

#define INVALID_MUSIC_ID 0xffffffff
#define MAX_MUSIC_ENTRIES 256

typedef uint32_t music_id_t;

extern music_id_t current_bgm;

music_id_t load_music(const char* path);

void init_music_handler();
void update_music_streams();
void play_music(music_id_t music);
void resume_music(music_id_t music);
void pause_music(music_id_t music);
void stop_music(music_id_t music);
void set_music_volume(music_id_t music, float volume);

#endif // MUSIC_HANDLER_H
