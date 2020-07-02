#include "player.h"

#include "draw/draw.h"
#include "draw/drawlist.h"
#include "draw/zorder.h"
#include "collision/collision.h"
#include "gameplay/gamestate.h"

#include "draw/spriteframe.h"
#include "draw/animation.h"

#include <raylib.h>

static spritesheet_id_t player_sheet;
static animation_t anim;
static animation_t trail_anim;

void init_player()
{
    player_sheet = load_spritesheet("resources/spritesheets/tobias.png", "player");

    anim.sheet = player_sheet;
    anim.anim_frame_count = 2;
    anim.start_frame = 0;
    anim.total_frametime = 8;

    anim.frames[0].spritesheet_rect = (rect_t){0, 0, 32, 32};
    anim.frames[0].frame_duration = 4;
    anim.frames[1].spritesheet_rect = (rect_t){32, 0, 32, 32};
    anim.frames[1].frame_duration = 4;

    trail_anim.sheet = player_sheet;
    trail_anim.anim_frame_count = 2;
    trail_anim.start_frame = 0;
    trail_anim.total_frametime = 8;

    trail_anim.frames[0].spritesheet_rect = (rect_t){0, 32, 32, 32};
    trail_anim.frames[0].frame_duration = 4;
    trail_anim.frames[1].spritesheet_rect = (rect_t){32, 32, 32, 32};
    trail_anim.frames[1].frame_duration = 4;
}

void update_player(float dt, player_t *player)
{
    if (player->inactive)
        return;

    bool focused = IsKeyDown(KEY_LEFT_SHIFT);
    float speed;
    if (focused)
        speed = player->focused_speed;
    else
        speed = player->unfocused_speed;

    vector2d_t new_pos = player->pos;

    // vertical movement
    {
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            new_pos.y += dt*speed;
        }
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            new_pos.y -= dt*speed;
        }
        if (new_pos.y < 0 + player->hitbox_radius)
            player->pos.y = 0.0f + player->hitbox_radius;
        else if (new_pos.y >= global_state.game_area_size.y - player->hitbox_radius)
            player->pos.y = global_state.game_area_size.y - player->hitbox_radius;
        else
            player->pos.y = new_pos.y;
    }
    // horizontal movement
    {
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            new_pos.x += dt*speed;
        }
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            new_pos.x -= dt*speed;
        }
        if (new_pos.x < 0 + player->hitbox_radius)
            player->pos.x = 0.0f + player->hitbox_radius;
        else if (new_pos.x >= global_state.game_area_size.x - player->hitbox_radius)
            player->pos.x = global_state.game_area_size.x - player->hitbox_radius;
        else
            player->pos.x = new_pos.x;
    }
}

bool test_player_collision(const player_t* player)
{
    if (player->inactive)
        return false;
    return test_collision(player->pos.x, player->pos.y, player->hitbox_radius);
}
bool test_player_graze(const player_t* player)
{
    if (player->inactive)
        return false;
    return false;
}

void draw_player_callback(void* player_ptr)
{
    const player_t* player = player_ptr;
    draw_circle(player->pos, 4.0, COL_BLUE);
    draw_animation(&anim, player->pos, (vector2d_t){32, 32}, 0.0);
    draw_animation(&trail_anim, (vector2d_t){player->pos.x, player->pos.y+32}, (vector2d_t){32, 32}, 0.0);
}

void draw_player(const player_t *player)
{
    register_draw_element((void*)player, draw_player_callback, PLAYER_ZORDER);
}
