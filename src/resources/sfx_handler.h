#ifndef SFX_HANDLER_H
#define SFX_HANDLER_H

#include "utils/resource_pool.h"

#define INVALID_SFX_ID INVALID_RES_ID
#define MAX_SFX_ENTRIES 1024

typedef res_id_t sfx_id_t;

sfx_id_t load_sfx(const char* path);

void init_sfx_handler();
void play_sfx(sfx_id_t music);
void set_sfx_volume(sfx_id_t id, float volume);

#endif // SFX_HANDLER_H
