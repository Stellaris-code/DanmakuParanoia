#include "gamestate.h"

#include "scripting/properties.h"
#include "scripting/syscalls.h"
#include "scripting/vm_drawlist.h"
#include "animation/asl_handler.h"
#include "bullet/bullet.h"
#include "bckgs/skybox.h"
#include "bckgs/3d_obj.h"
#include "draw/vfx.h"
#include "sys/time.h"
#include "sys/cleanup.h"
#include "sys/log.h"
#include "engine/engine.h"
#include "utils/benchmark.h"
#include "utils/timed_action.h"
#include "draw/particles.h"
#include "draw/zorder.h"
#include "draw/drawlist.h"
#include "draw/draw.h"
#include "ui/hud.h"

gameplay_state_t global_state;

property_value_t test_prop_random()
{
    return (property_value_t){.ival = (rand() % 1024)};
}

sprite_frame_id_t sprite;
sprite_frame_id_t shard_sprite;

void gamestate_init(state_t *state)
{
    gameplay_state_t* gamestate = (gameplay_state_t*)state;

    gamestate->current_frame = 0;
    gamestate->player.focused_speed = 80.0f;
    gamestate->player.unfocused_speed = 300.0f;
    gamestate->player.hitbox_radius = 1.5f;
    gamestate->player.pos.x = 400;
    gamestate->player.pos.y = 450 - 2;
    gamestate->player_dead = 0;
    gamestate->player.inactive = false;

    gamestate->current_bgm = INVALID_MUSIC_ID;
    gamestate->freeze = false;
    gamestate->timescale = 1;

    init_bullet_manager();

    init_player();

    SetCameraMoveControls(0, 0, 0, 0, 0, 0); // no camera keyboard control

    // ~2ms
    gamestate->death_sound = load_sfx("resources/sfx/deathsound.mp3");

    printf("sound mem : %d\n", danpa_allocated_mem());

    double bob = elapsed_time();
    gamestate->sprite_atlas  = load_spritesheet("resources/spritesheets/th06spritesheet_alpha_jp2.png", "danmaku-sheet-1");

    global_state.sprite_atlas = gamestate->sprite_atlas; // TODO UGLY TEMPORARY
    //rect_t rect = {.x = 364, .y = 62, .w = 16, .h = 16};
    rect_t rect = {.x = 462, .y = 227, .w = 32, .h = 32};
    sprite = load_sprite_frame(gamestate->sprite_atlas, rect);
    rect = (rect_t){.x = 384, .y = 96, .w = 8, .h = 16};
    shard_sprite = load_sprite_frame(gamestate->sprite_atlas, rect);

    sprite_frame_id_t particles_sheet = load_spritesheet("resources/spritesheets/particles.png", "particles-sheet");
    printf("sheets mem : %d\n", danpa_allocated_mem());

    double diff = elapsed_time() - bob;
    printf("------  Textures loading time : %02.03f\n", diff*1000);

    SPAWN_VFX(0, sheared_rectangle,
              (vector2d_t){200.f, 000.f}, COL_PINK, 20.f, -0.25f, 0.0f, 1000.f, 1.f, false);
    SPAWN_VFX(0, sheared_rectangle,
              (vector2d_t){800.f, -500.f}, COL_PINK, 20.f, -0.25f, 0.0f, 1000.f, 1.2f, false);
    SPAWN_VFX(0, sheared_rectangle,
              (vector2d_t){400.f, -1500.f}, COL_PINK, 30.f, -0.25f, 0.0f, 2000.f, 1.5f, false);
    printf("vfx mem : %d\n", danpa_allocated_mem());
    register_script_property("CirnoAnswer", (script_property_t){
                                 .is_constant = true,
                                 .type = PropertyString,
                                 .const_val.str = "There are no buses in Gensokyo!"
                             });
    register_script_property("screen_w", (script_property_t){
                                 .is_constant = true,
                                 .type = PropertyInt,
                                 .const_val.ival = gamestate->game_area_size.x
                             });
    register_script_property("screen_h", (script_property_t){
                                 .is_constant = true,
                                 .type = PropertyInt,
                                 .const_val.ival = gamestate->game_area_size.y
                             });

    register_script_property("Random", (script_property_t){
                                 .is_constant = false,
                                 .type = PropertyInt,
                                 .getter = &test_prop_random
                             });
    printf("cam mem : %d\n", danpa_allocated_mem());
    gamestate->camera = (Camera){ { 1.0f, 1.0f, 1.0f }, { 4.0f, 1.0f, 4.0f }, { 0.0f, 1.0f, 0.0f }, 40.0f, 0 };
    SetCameraMode(gamestate->camera, CAMERA_FIRST_PERSON);

    bob = elapsed_time();
    load_skybox("resources/skybox.png");
    diff = elapsed_time() - bob;
    printf("------  Skybox loading time : %02.03f\n", diff*1000);

    printf("skybox mem : %d\n", danpa_allocated_mem());

    // shader loading time : ~ 3
    gamestate->cool_id = load_shader("resources/shaders/death_shader.fs", NormalShader);
    gamestate->transition_shader = load_shader("resources/shaders/circle_transition.fs", NormalShader);
    gamestate->grayscale_shader = load_shader("resources/shaders/grayscale.fs", PostFXShader);

    // VM init time : ~0.5ms
    printf("shader mem : %d\n", danpa_allocated_mem());
    gamestate->vm = create_vm("in.bin");
    if (!gamestate->vm)
    {
        trace_log(LOG_ERROR, "Could not load the vm script");
        return;
    }
    init_scripting_syscalls(gamestate->vm);
    vm_run_init(gamestate->vm);
    printf("vm mem : %d\n", danpa_allocated_mem());
}

void gamestate_exit(state_t *state)
{
    gameplay_state_t* gamestate = (gameplay_state_t*)state;

    cleanup_vm(gamestate->vm);
}

static void start_transition(void* arg)
{
    enable_shader(*(int*)arg, 2.0f, ABOVEALL_ZORDER, true);
}

static void gamestate_reset()
{
    do_cleanup(GamestateEnd);
}

static void handle_events(gameplay_state_t* gamestate)
{
    if (IsKeyPressed(KEY_Q))
    {
        gamestate_reset();
    }
    if(IsKeyPressed(KEY_P))
    {
        gamestate->freeze = !gamestate->freeze;
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
}

void gamestate_update(state_t *state, float dt)
{
    gameplay_state_t* gamestate = (gameplay_state_t*)state;
    ++gamestate->current_frame;
    global_state.current_frame = gamestate->current_frame; // TODO UGLY REMOVE

    float frame_start_time = GetTime();
    dt *= gamestate->timescale; // fixed timestep

    bench_begin_frame();

    START_BENCHMARK(events);
    handle_events(gamestate);
    END_BENCHMARK(events, "Event handling");

    START_BENCHMARK(game_logic);
    float game_logic_start_time = GetTime();
    vm_drawlist_reset();
    update_music_streams();
    vfx_update(dt);
    update_shader_fx(dt);
    update_player(dt, &gamestate->player);
    global_state.player = gamestate->player; // UGLY TEMPORARY
    if (!gamestate->freeze)
    {
        update_asl_scripts(dt);
    }
    UpdateCamera(&gamestate->camera);
    END_BENCHMARK(game_logic, "Game Logic");
    START_BENCHMARK(bullet_update);
    if (!gamestate->freeze)
    {
        update_bullets(dt);
    }
    END_BENCHMARK(bullet_update, "Bullet update");

    float vm_start_time = GetTime();
    START_BENCHMARK(vm_run);
    if (!gamestate->freeze)
    {
        vm_reset(gamestate->vm);
        vm_run(gamestate->vm);
    }
    END_BENCHMARK(vm_run, "VM Execution");
    // TODO : configurable GC execution threshold
    START_BENCHMARK(vm_gc);
    if (1 || gamestate->vm->allocated_memory_region_count >= MAX_MEMORY_REGIONS/2)
        vm_run_gc(gamestate->vm);
    END_BENCHMARK(vm_gc, "VM GC");

    START_BENCHMARK(collisions);
    float collision_start_time = GetTime();
    global_state.collision = test_player_collision(&gamestate->player);
    END_BENCHMARK(collisions, "Collisions");

    bool player_collision = false;
    //player_collision = global_state.collision;

    START_BENCHMARK(particles);
    float particles_start_time = GetTime();
    update_particle_system(dt);
    END_BENCHMARK(particles, "Particles");

    //return false;
}

void gamestate_render(state_t *state)
{
    gameplay_state_t* gamestate = (gameplay_state_t*)state;

    bool player_collision = false;
    //player_collision = global_state.collision;

    if (!gamestate->player_dead && player_collision)
    {
        //TakeScreenshot("death.png");

        global_state.player_dead = true;
        gamestate->death_frame = engine.current_frame;
        gamestate->death_time = GetTime();
        global_state.player.inactive = true;
        schedule_action(1.20f, start_transition, &gamestate->transition_shader);
        schedule_action(2.25f, gamestate_reset, NULL);
        play_sfx(gamestate->death_sound);

        enable_shader(gamestate->cool_id, 2.0f, UNDER_PLAYER_ZORDER, false);
        enable_shader(gamestate->grayscale_shader, 1.0f, BELOWALL_ZORDER, true);

        SPAWN_VFX(0, screen_shake, 35, 35, 160.f, 0.5f);
    }

    color_t bg_color = {0xff, 0xff, 0xff, 0xff};
    if (global_state.player_dead)
    {
        if (gamestate->current_bgm != INVALID_MUSIC_ID)
            set_music_volume(gamestate->current_bgm, 1.0f - fminf((engine.current_frame-gamestate->death_frame)/50.f, 1.0f));
    }
    Color rl_bgcol;
    rl_bgcol.r = bg_color.r; rl_bgcol.g = bg_color.g; rl_bgcol.b = bg_color.b; rl_bgcol.a = bg_color.a;

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(rl_bgcol);

    START_BENCHMARK(backgrounds);
    float bckg_start_time = GetTime();
    BeginMode3D(gamestate->camera);
    draw_skybox();
    draw_all_obj3d();
    EndMode3D();
    END_BENCHMARK(backgrounds, "Backgrounds");

    START_BENCHMARK(drawlist_issues);
    vfx_draw();
    draw_shader_fx();

    if (gamestate->player_dead && (engine.current_frame-gamestate->death_frame)>60)
    {
        translate_viewport((vector3d_t){0.0, -expf((engine.current_frame-gamestate->death_frame-60)/10.f), 0.0});
    }

    draw_bullets();
    draw_particles();

    if (!gamestate->player_dead)
        draw_player(&gamestate->player);
    END_BENCHMARK(drawlist_issues, "Drawlist issues");

    // to handle z-ordered rendering
    START_BENCHMARK(render2d);
    float render2d_start_time = GetTime();
    sort_drawlist();
    commit_drawlist();
    END_BENCHMARK(render2d, "2D Drawcalls");

    // HUD stuff : ignore previous transforms
    reset_viewport();

    bench_end_frame();
    //draw_hud();

    rect_t game_area_rect;
    game_area_rect.x = game_area_rect.y = 0;
    game_area_rect.w = global_state.game_area_size.x;
    game_area_rect.h = global_state.game_area_size.y;

    EndDrawing();
    //----------------------------------------------------------------------------------
}
