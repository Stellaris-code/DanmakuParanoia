#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "math/vector.h"

#include "gameplay/player.h"

typedef struct gamestate_t
{
    ivector2d_t game_area_size;
    int current_frame;
    player_t player;
    bool player_dead;

    // stats
    float busy_frame_time;
    float vm_frame_time;
    float collision_frame_time;
    float bckg_frame_time;
    float render2d_frame_time;
    float game_logic_frame_time;
    float particles_frame_time;
} gamestate_t;

extern gamestate_t global_state;

#endif // GAMESTATE_H
