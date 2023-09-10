/*******************************************************************************************
*
*   raylib [text] example - Text formatting
*
*   This example has been created using raylib 1.1 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

#include "engine/engine.h"

#include "math/vector.h"

#include "utils/utils.h"
#include "utils/constants.h"
#include "utils/benchmark.h"
#include "sys/cleanup.h"
#include "sys/log.h"
#include "sys/support.h"

#include "scripting/properties.h"
#include "animation/asl_handler.h"
#include "draw/particles.h"
#include "utils/timed_action.h"
#include "draw/vfx.h"
#include "bckgs/skybox.h"
#include "bckgs/3d_obj.h"

#include "engine/engine.h"
#include "gameplay/gamestate.h"

#include "thread_support.h"

#include "vm.h"

#define _GNU_SOURCE
#include <math.h>

#include <stdio.h>
#include <assert.h>
#include <time.h>

// CONVENTION :
// angles in radians
// distance unit : pixel

// TODOs majeurs :
// TODO TODO TODO : fix that ugly "global_state" thing
// Draw hitbox du joueur au dessus des bullet
// Ouvrir issue sur Raylib ligne 196 text.c : calloc au lieu de RL_CALLOC
// Regarder si l'issue avec LoadOBJ a été résolue
// Error handling in scripting VM
// Améliorer la gestion des ajustement de viewport ('translate_viewport' et compagnie)
// Utiliser un système de gamestates
// Ajouter de l'error handling à l'ASL
// Script : a way to attach coroutines to bullets
// TODO : custom assert()
// TODO : measure if fast_sincos is accurate enough for pixel-perfect collisions
// TODO : moyen d'afficher les hitbox pour le debug
// TODO : cross platform simple SIMD operations
// TODO : investigate stuttery bullet motion
// TODO : measure actual GPU time
// TODO : script tasks (coroutines)
// TODO : implement 'eql', 'lel', 'gtl'... all binops operating on local variables in the compiler and assembler
// TODO : remplacer sincos dans motion_t par des rotations par nombres complexes
// TODO : 'for (int i = 0; i < (int)(360/4/1.5); ++i)' causes VM error (invalid cmp types)
// TODO : virer tous les fast_sincos immondes
// Some texture atlas bleeding (texel coordinates?)
// Est-ce que la méthode de Wait de fin de frame est optimale? busy loop ou sleep?
// Ajouter une console ?
// Ajouter un système de stats par res_pool
// dans syscalls.c : // TODO : use something better than ugly extern references

/*
void spawn_vfx_2(void* arg);
void spawn_vfx_1(void* arg)
{
    static float offset = 50.f;
    static float delay = 0.5f;
    static int max_rects = 50;

    SPAWN_VFX(1, sheared_rectangle,
              (vector2d_t){offset, 0.f}, COL_PINK, 20.f, -0.25f, 0.0f, 1000.f, 1.0f, false);
    SPAWN_VFX(0, sheared_rectangle,
              (vector2d_t){offset + 15.f, 0.f + 15.f}, COL_LITERAL(color_t){ 255/2, 109/2, 194/2, 255 }, 20.f, -0.25f, 0.0f, 1000.f, 1.0f, false);
    offset += 80.f;

    --max_rects;
    if (max_rects < 0)
        return; // don't spawn more rects

    if (offset >= 800.f + 100.f)
    {
        schedule_action(0.2f, spawn_vfx_2, NULL);
        offset = 50.f;
    }

    schedule_action(delay, spawn_vfx_1, NULL);
    delay *= 0.75f;
}

void spawn_vfx_2(void* arg)
{
    static float offset = -200.f;
    static int foo = 0;

    SPAWN_VFX(2, sheared_rectangle,
              (vector2d_t){00.0f, offset}, foo ? COL_GRAY : COL_DARKGRAY, 100.f, -0.50f, 1.92f, -1400.f, 0.8f, true);

    foo = !foo;

    offset += 200.f;
*/

void compile_scripts()
{
    system("\"\"../DanPaCompiler/build-DanPaCompiler_local-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/DanPaCompiler.exe\" \"build-DanmakuParanoia-master-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/script.dps\" \"script.asm\"\"");
    system("\"\"../build-DanPaAssembler-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/DanPaAssembler.exe\""
           " \"C:/Users/Ya2nb/Travail/C++/DanPaGit/Projets C++/build-DanmakuParanoia-master-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/script.asm\" \"build-DanmakuParanoia-master-Static-Debug/in.bin\" \"");
}

// TODO : warn about issue if .obj file normals are not written
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    srand(time(0));

    compile_scripts();

    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    //SetConfigFlags(FLAG_VSYNC_HINT);

#if 1
    unsigned res_x = 1200 / 2;
    unsigned res_y = 800;
#else
    unsigned res_x = 1920;//2560;
    unsigned res_y = 1080;//1440;
#endif

    SetTraceLogLevel(LOG_WARNING);

    //InitWindow(res_x, res_y, "弾幕パラノイア");
    InitWindow(res_x, res_y, "th21.37 - Imperishable Nöt");
    InitAudioDevice();

    SetTargetFPS(TARGET_FPS);               // Set our game to run at 60 frames-per-second
    //SetTargetFPS(0);

    engine_init(&engine);

    gameplay_state_t gamestate;
    gamestate.base.exit = gamestate_exit;
    gamestate.base.init = gamestate_init;
    gamestate.base.update = gamestate_update;
    gamestate.base.render = gamestate_render;

    engine.render_area_size = (ivector2d_t){res_x, res_y};
    gamestate.game_area_size = engine.render_area_size;
    global_state.game_area_size = engine.render_area_size; // TODO UGLY GET RID OF "global_state"

    init_timed_actions();
    init_music_handler();
    init_sfx_handler();
    init_texture_handler();
    init_skybox();
    init_obj3ds();
    init_vfx();
    init_shader_fx();
    init_textures();
    init_particle_system();
    init_properties();
    init_asl_scripting();

    // TODO : make it appear in a graphical popup message (animation: perhaps one that expands?)
    const char* err_msg;
    if ((err_msg = test_cpu_os_support()) != NULL)
    {
        trace_log(LOG_FATAL, "CPU/OS error : %s\nAborting", err_msg);
        return -1;
    }
    trace_log(LOG_INFO, "Available hardware threads : %d", thrd_hardware_threads());

    engine_push_state(&engine, &gamestate.base);

    //--------------------------------------------------------------------------------------

    engine_run_loop(&engine);

    engine_cleanup(&engine);

    do_cleanup(AppEnd);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseAudioDevice();

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
