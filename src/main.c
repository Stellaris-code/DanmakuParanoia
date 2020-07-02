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

#include "gameplay/bullet/bullet.h"
#include "gameplay/player.h"
#include "gameplay/gamestate.h"
#include "gameplay/entity.h"
#include "collision/collision.h"

#include "resources/texture_handler.h"
#include "resources/music_handler.h"
#include "draw/draw.h"
#include "draw/vfx.h"
#include "draw/animation.h"
#include "draw/shader_fx.h"
#include "draw/drawlist.h"
#include "draw/zorder.h"
#include "draw/particles.h"
#include "bckgs/3d_obj.h"
#include "bckgs/skybox.h"
#include "scripting/syscalls.h"
#include "scripting/vm_drawlist.h"

#include "math/vector.h"

#include "ui/hud.h"
#include "utils/timed_action.h"
#include "utils/utils.h"
#include "sys/cleanup.h"

#include "vm.h"

#define _GNU_SOURCE
#include <math.h>

#include <stdio.h>
#include <assert.h>
#include <time.h>

#define TARGET_FPS 60

// CONVENTION :
// angles in radians
// distance unit : pixel


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
}

void start_transition(void* arg)
{
    enable_shader(*(int*)arg, 2.0f, ABOVEALL_ZORDER, true);
}

vm_state_t* test_vm;
int death_frame;
float death_time;
float frame_time;
sprite_frame_id_t sprite;
sprite_frame_id_t shard_sprite;
sprite_frame_id_t particles_sheet;
void reset();
void handle_events()
{
    if (IsKeyPressed(KEY_Q))
    {
        reset();
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        DrawText(TextFormat("Mouse Pos : %i, %i", GetMouseX(), GetMouseY()), 200, 40, 20, BLACK);

        for (int i = 0; i < 0x1000*25; ++i)
        {
            bullet_t bullet = {0};
            bullet.type = Circle;
            bullet.sprite = sprite;
            bullet.motion.direction_angle = M_PI_4;
            bullet.motion.velocity = 400.f;
            bullet.motion.angular_velocity = 4.f;
            bullet.motion.relative_x = 400.f;
            bullet.motion.relative_y = 225.f;
            circle_info_t info = {0};
            info.radius = 10.f;
            info.hitbox_radius = 5.f;

            register_bullet(bullet, &info);
        }
    }
    if (IsKeyPressed(KEY_B))
    {
        DrawText(TextFormat("Mouse Pos : %i, %i", GetMouseX(), GetMouseY()), 200, 40, 20, BLACK);

        for (int i = 0; i < 0x1; ++i)
        {
            bullet_t bullet = {0};
            bullet.type = Rect;
            bullet.sprite = shard_sprite;
            bullet.motion.direction_angle = M_PI_4;
            //bullet.motion.velocity = 400.f;
            //bullet.motion.angular_acceleration = 4.f;
            //bullet.motion.rotational_speed = 50.f/20.f;
            bullet.motion.rotation = M_PI_4;
            bullet.motion.relative_x = 400.f;
            bullet.motion.relative_y = 225.f;
            rect_info_t info = {0};
            info.width = 16.f;
            info.height = 18.f;
            info.hitbox_width = 6.f;
            info.hitbox_height = 12.f;

            bullet_t* root_ptr = register_bullet(bullet, &info);

            bullet_t satellite = {0};
            satellite.type = Circle;
            satellite.sprite = sprite;
            satellite.motion.direction_angle = M_PI_2; // looking up
            satellite.motion.velocity = 50.f;
            satellite.motion.angular_velocity = 50.f/20.f; // omega=V/R in radians
            satellite.motion.relative_x = 20.f;
            satellite.motion.relative_y = 00.f;
            satellite.motion.root = &root_ptr->motion;
            circle_info_t satellite_info = {0};
            satellite_info.radius = 10.f;

            //register_bullet(satellite, &satellite_info);

            // another one
            satellite = (bullet_t){0};
            satellite.type = Circle;
            satellite.sprite = sprite;
            satellite.motion.direction_angle = M_PI_2; // looking up
            satellite.motion.velocity = 140.f;
            satellite.motion.angular_velocity = 140.f/40.f; // omega=V/R in radians
            satellite.motion.relative_x = 40.f;
            satellite.motion.relative_y = 00.f;
            satellite.motion.root = &root_ptr->motion;
            satellite_info = (circle_info_t){0};
            satellite_info.radius = 10.f;

            //register_bullet(satellite, &satellite_info);
        }
    }
    //if (false&&IsKeyPressed(KEY_S))
    {
        float vm_start_time = GetTime();
        vm_reset(test_vm);
        vm_run(test_vm);
        if (test_vm->allocated_region_count >= MAX_MEMORY_REGIONS/2)
            vm_run_gc(test_vm);
        global_state.vm_frame_time = GetTime() - vm_start_time;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        DrawCircle(GetMouseX(), GetMouseY(), 10, GREEN);
        global_state.player.pos.x = GetMouseX();
        global_state.player.pos.y = GetMouseY();
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        SPAWN_VFX(0, screen_shake, 10, 10, 120.f, 0.5f);
    }

    if (IsKeyPressed(KEY_F))
    {
        emitter_t* emitter = create_emitter();
        emitter->lifetime = 5.0f;
        emitter->archetype_count = 1;
        emitter->archetype_strategy = LOOP;
        emitter->particles_sheet = particles_sheet;
        emitter->zorder = BELOWALL_ZORDER;
        emitter->blend_mode = BlendAdditive;
        emitter->frequency = 5.0f;
        emitter->burst_size = 2;
        emitter->motion = (motion_data_t){0};
        emitter->motion.relative_x = global_state.game_area_size.x/2.0;
        emitter->motion.relative_y = 100.0;
        emitter->motion_randomness.direction_angle = 2.f*M_PI;
        emitter->motion_randomness.velocity = 20.f;

        particle_t particle = {0};
        particle.initial_color = COL_GRAY; particle.initial_color.a = 200;
        particle.final_color = COL_RED; particle.final_color.a = 0;
        particle.initial_scale = (vector2d_t){0.25f, 0.25f};
        particle.final_scale = (vector2d_t){0.25f, 0.25f};
        particle.lifetime = 2.f;
        particle.motion.rotational_speed = M_PI;
        particle.motion.velocity = 20.f;
        particle.motion.max_rot = particle.motion.max_accel = particle.motion.max_speed =
            particle.motion.max_angular = +INFINITY;
        rect_t sprite_rect;
        sprite_rect.x = 8*512; sprite_rect.y = 8*512;
        sprite_rect.w = sprite_rect.h = 512;
        particle.spriteframe = load_sprite_frame(particles_sheet, sprite_rect);

        emitter->particle_archetypes[0] = particle;
    }
}

void compile_scripts()
{
    system("\"D:/Compiegne C++/Projets C++/DanPaCompiler/DanPaCompiler/build/DanPaCompiler.exe\"");
    system("\"D:/Compiegne C++/Projets C++/DanPaAssembler/build/DanPaAssembler.exe\"");
}

void reset()
{
    if (current_bgm != INVALID_MUSIC_ID)
        stop_music(current_bgm);

    clear_bullets();
    cleanup_obj3ds();
    clear_entities();
    clear_particles();
    global_state.current_frame = global_state.current_frame = 0;
    global_state.player_dead = 0;
    global_state.player.inactive = false;
    global_state.player.pos.x = 400;
    global_state.player.pos.y = 450 - 2;
    vm_clear(test_vm);
    vm_run_init(test_vm);
}

// TODO : warn about issue if .obj file normals are not written
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    srand(time(0));

    float timescale = 1.0f;

    compile_scripts();

    test_vm = create_vm("in.bin");
    init_scripting_syscalls(test_vm);
    vm_run_init(test_vm);

    global_state.player.focused_speed = 80.0f;
    global_state.player.unfocused_speed = 300.0f;
    global_state.player.hitbox_radius = 2.0f;
    global_state.player.pos.x = 400;
    global_state.player.pos.y = 450 - 2;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 450, "raylib [text] example - 弾幕パラノイア");

    InitAudioDevice();

    SetTargetFPS(TARGET_FPS);               // Set our game to run at 60 frames-per-second

    global_state.game_area_size = (ivector2d_t){800, 450};

    Sound death_sound = LoadSound("resources/sfx/deathsound.mp3");

    init_music_handler();
    init_texture_handler();
    init_skybox();
    init_obj3ds();
    init_vfx();
    init_shader_fx();
    init_textures();
    init_player();
    init_particle_system();

    spritesheet_id_t sheet   = load_spritesheet("resources/spritesheets/th06spritesheet_alpha.png", "danmaku-sheet-1");
    rect_t rect = {.x = 364, .y = 62, .w = 16, .h = 16};
    sprite = load_sprite_frame(sheet, rect);
    rect = (rect_t){.x = 380, .y = 95, .w = 16, .h = 18};
    shard_sprite = load_sprite_frame(sheet, rect);
    particles_sheet = load_spritesheet("resources/spritesheets/particles.png", "particles-sheet");

    SPAWN_VFX(0, sheared_rectangle,
              (vector2d_t){200.f, 000.f}, COL_PINK, 20.f, -0.25f, 0.0f, 1000.f, 1.f, false);
    SPAWN_VFX(0, sheared_rectangle,
              (vector2d_t){800.f, -500.f}, COL_PINK, 20.f, -0.25f, 0.0f, 1000.f, 1.2f, false);
    SPAWN_VFX(0, sheared_rectangle,
              (vector2d_t){400.f, -1500.f}, COL_PINK, 30.f, -0.25f, 0.0f, 2000.f, 1.5f, false);

    Camera camera = { { 1.0f, 1.0f, 1.0f }, { 4.0f, 1.0f, 4.0f }, { 0.0f, 1.0f, 0.0f }, 40.0f, 0 };
    SetCameraMode(camera, CAMERA_FIRST_PERSON);

    load_skybox("resources/skybox.png");

    int cool_id = load_shader("resources/shaders/death_shader.fs", NormalShader);
    int transition_shader = load_shader("resources/shaders/circle_transition.fs", NormalShader);
    int grayscale_shader = load_shader("resources/shaders/grayscale.fs", PostFXShader);

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        float frame_start_time = GetTime();
        //const float dt = GetFrameTime();
        const float dt = (1.0f/TARGET_FPS) * timescale; // fixed timestep
        frame_time = dt;

        float collision_start_time = GetTime();
        bool player_collision = test_player_collision(&global_state.player);
        global_state.collision_frame_time = GetTime() - collision_start_time;

        float game_logic_start_time = GetTime();
        vm_drawlist_reset();
        update_music_streams();
        timed_actions_update(dt);
        vfx_update(dt);
        update_shader_fx(dt);
        update_player(dt, &global_state.player);
        update_bullets(dt);
        UpdateCamera(&camera);
        global_state.game_logic_frame_time = GetTime() - game_logic_start_time;

        float particles_start_time = GetTime();
        update_particle_system(dt);
        global_state.particles_frame_time = GetTime() - particles_start_time;

        if (!global_state.player_dead && player_collision)
        {
            //TakeScreenshot("death.png");

            global_state.player_dead = true;
            death_frame = global_state.current_frame;
            death_time = GetTime();
            global_state.player.inactive = true;
            schedule_action(1.20f, start_transition, &transition_shader);
            schedule_action(2.25f, reset, NULL);
            PlaySound(death_sound);

            enable_shader(cool_id, 2.0f, UNDER_PLAYER_ZORDER, false);
            enable_shader(grayscale_shader, 1.0f, BELOWALL_ZORDER, true);

            SPAWN_VFX(0, screen_shake, 35, 35, 160.f, 0.5f);
        }

        color_t bg_color = {0xff, 0xff, 0xff, 0xff};
        if (global_state.player_dead)
        {
            if (current_bgm != INVALID_MUSIC_ID)
                set_music_volume(current_bgm, 1.0f - fminf((global_state.current_frame-death_frame)/50.f, 1.0f));
        }
        Color rl_bgcol;
        rl_bgcol.r = bg_color.r; rl_bgcol.g = bg_color.g; rl_bgcol.b = bg_color.b; rl_bgcol.a = bg_color.a;

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(rl_bgcol);

        float bckg_start_time = GetTime();
        BeginMode3D(camera);
            draw_skybox();
            draw_all_obj3d();
        EndMode3D();
        global_state.bckg_frame_time = GetTime() - bckg_start_time;

        vfx_draw();
        draw_shader_fx();

        if (global_state.player_dead && (global_state.current_frame-death_frame)>60)
        {
            translate_viewport((vector3d_t){0.0, -expf((global_state.current_frame-death_frame-60)/10.f), 0.0});
        }

        handle_events();

        draw_bullets();
        draw_particles();

        if (!global_state.player_dead)
            draw_player(&global_state.player);

        // to handle z-ordered rendering
        float render2d_start_time = GetTime();
        sort_drawlist();
        commit_drawlist();
        global_state.render2d_frame_time = GetTime() - render2d_start_time;

        // HUD stuff : ignore previous transforms
        reset_viewport();

        draw_hud();

        global_state.busy_frame_time = GetTime() - frame_start_time;

        /*
        int id = test_collision(GetMouseX(), GetMouseY(), 10);
        if (id)
        {
            DrawText(FormatText("Collision test : %d", id), 10, 260, 20, BLACK);
        }
        */

        rect_t game_area_rect;
        game_area_rect.x = game_area_rect.y = 0;
        game_area_rect.w = global_state.game_area_size.x;
        game_area_rect.h = global_state.game_area_size.y;

        EndDrawing();
        //----------------------------------------------------------------------------------

        ++global_state.current_frame;
    }

    do_cleanup();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    cleanup_vm(test_vm);

    return 0;
}
