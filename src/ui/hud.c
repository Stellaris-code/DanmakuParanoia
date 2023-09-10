#include "hud.h"

#include <rlgl.h>

#include "gameplay/bullet/bullet.h"
#include "gameplay/gamestate.h"
#include "draw/particles.h"
#include "sys/alloc.h"
#include "utils/benchmark.h"
#include "engine/engine.h"

static int stat_y_base;

void push_stat_text_color(const char* str, Color tint)
{
    DrawText(str, 10, stat_y_base, 20, tint);

    stat_y_base += 20;
}

void push_stat_text(const char* str)
{
    push_stat_text_color(str, BLACK);
}

void draw_hud()
{
    // TODO : death animation : draw a heart being broken with a negative color shader (~1s)
    // each death breaks it a little further, until no more lives are left
    if (global_state.player_dead && ((engine.current_frame)/8 % 4 >= 1))
    {
        int width = MeasureText("Stay Determined...", 40);
        DrawText(TextFormat("Stay Determined..."), 400-width/2, 125, 40, RED);
    }

    stat_y_base = 20;

    DrawRectangle(10, 10, 400, 360, CLITERAL(Color){ 200, 200, 200, 200 });

    push_stat_text(TextFormat("Elapsed Time: %02.02f/%02.02f ms (FPS: %d)", bench_busy_time*1000, GetFrameTime()*1000, (int)(1.0/bench_busy_time)));
    push_stat_text_color(TextFormat("Margin : %02.03f ms", 1/60.f*1000.f - bench_busy_time*1000), bench_busy_time*1000 > 1/60.f*1000.f ? RED : BLACK);
    float sum = 0;
    for (unsigned i = 0; i < bench_entries_count; ++i)
    {
        sum += bench_entries[i].time;
        push_stat_text(TextFormat("%s: %02.03f ms", bench_entries[i].module_name, bench_entries[i].time*1000));
    }
    push_stat_text(TextFormat("Other: %02.03f ms", (bench_busy_time-sum)*1000));
    push_stat_text(TextFormat("Bullets : %d", total_bullet_count()));
    push_stat_text(TextFormat("Particles : %d", particle_count));
    push_stat_text(TextFormat("Committed memory : %lld\n", danpa_allocated_mem()));
    push_stat_text(TextFormat("Allocations this frame : %d\n", danpa_alloc_count));
    if (global_state.collision)
        push_stat_text_color("Collision", RED);

    DrawFPS(10, 10);

    danpa_alloc_count = 0;
}
