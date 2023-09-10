#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "raylib.h"

#include "engine/state.h"

#include "math/vector.h"
#include "gameplay/player.h"
#include "draw/shader_fx.h"

#include "vm.h"

#include "resources/sfx_handler.h"
#include "resources/texture_handler.h"
#include "resources/music_handler.h"

typedef struct gameplay_state_t
{
    state_t base;

    ivector2d_t game_area_size;
    int current_frame;
    int death_frame;
    float death_time;
    player_t player;
    float timescale;
    bool collision;
    bool player_dead;
    bool freeze;

    vm_state_t* vm;

    sfx_id_t death_sound;
    spritesheet_id_t sprite_atlas;

    music_id_t current_bgm;

    shader_fx_id_t cool_id;
    shader_fx_id_t transition_shader;
    shader_fx_id_t grayscale_shader;

    Camera camera;
} gameplay_state_t;

void gamestate_init(state_t* state);
void gamestate_exit(state_t* state);
void gamestate_update(state_t* state, float dt);
void gamestate_render(state_t* state);

extern gameplay_state_t global_state;

#endif // GAMESTATE_H
