#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#include "draw/spriteframe.h"
#include "math/vector.h"

#define GRAZE_RADIUS 10.f

typedef struct player_t
{
    sprite_frame_t sprite;
    vector2d_t pos;
    float hitbox_radius;
    float focused_speed;
    float unfocused_speed;
    bool inactive; // i.e. : dead, or respawning
} player_t;

void init_player();
void update_player(float dt, player_t* player);
void draw_player(const player_t* player);

bool test_player_collision(const player_t* player);
bool test_player_graze(const player_t* player);

#endif // PLAYER_H
