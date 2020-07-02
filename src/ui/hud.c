#include "hud.h"

#include <rlgl.h>

#include "gameplay/bullet/bullet.h"
#include "gameplay/gamestate.h"
#include "draw/particles.h"

void draw_hud()
{
    // TODO : death animation : draw a heart being broken with a negative color shader (~1s)
    // each death breaks it a little further, until no more lives are left
    if (global_state.player_dead && ((global_state.current_frame)/8 % 4 >= 1))
    {
        int width = MeasureText("Stay Determined...", 40);
        DrawText(FormatText("Stay Determined..."), 400-width/2, 125, 40, RED);
    }

    float sum = global_state.vm_frame_time + global_state.collision_frame_time + global_state.bckg_frame_time +
        global_state.render2d_frame_time + global_state.game_logic_frame_time + global_state.particles_frame_time;

    DrawText(FormatText("Elapsed Time: %02.02f/%02.02f ms", global_state.busy_frame_time*1000, GetFrameTime()*1000), 10, 40, 20, BLACK);
    DrawText(FormatText("Busy Frame Time: %02.02f ms", global_state.busy_frame_time*1000), 10, 60, 20, BLACK);
    DrawText(FormatText("Script: %02.02f ms", global_state.vm_frame_time*1000), 10, 80, 20, BLACK);
    DrawText(FormatText("Collision: %02.02f ms", global_state.collision_frame_time*1000), 10, 100, 20, BLACK);
    DrawText(FormatText("3D Background: %02.02f ms", global_state.bckg_frame_time*1000), 10, 120, 20, BLACK);
    DrawText(FormatText("2D Draw: %02.02f ms", global_state.render2d_frame_time*1000), 10, 140, 20, BLACK);
    DrawText(FormatText("Game Logic: %02.02f ms", global_state.game_logic_frame_time*1000), 10, 160, 20, BLACK);
    DrawText(FormatText("Particles: %02.02f ms", (global_state.particles_frame_time)*1000), 10, 180, 20, BLACK);
    DrawText(FormatText("Other: %02.02f ms", (global_state.busy_frame_time-sum)*1000), 10, 200, 20, BLACK);
    DrawText(FormatText("Bullet Count : %d", total_bullet_count()), 10, 220, 20, BLACK);
    DrawText(FormatText("Particles : %d", particle_count), 10, 240, 20, BLACK);

    DrawFPS(10, 10);
}
