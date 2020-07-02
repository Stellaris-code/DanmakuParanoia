#include "syscalls.h"

#include <math.h>

#include "math/easing.h"
#include "gameplay/motion.h"
#include "gameplay/player.h"
#include "gameplay/gamestate.h"
#include "gameplay/entity.h"
#include "gameplay/bullet/bullet.h"
#include "resources/texture_handler.h"
#include "resources/music_handler.h"
#include "draw/draw.h"
#include "draw/animation.h"
#include "draw/particles.h"
#include "bckgs/3d_obj.h"
#include "scripting/vm_drawlist.h"

// TODO : use something better than ugly extern references

extern float frame_time;
extern sprite_frame_id_t sprite;
extern sprite_frame_id_t shard_sprite;

void player_pos_syscall(vm_state_t* state)
{
    var_t ptr_y = pop_stack(state);
    var_t ptr_x = pop_stack(state);

    vm_store(state, ptr_x, MK_FLT(global_state.player.pos.x));
    vm_store(state, ptr_y, MK_FLT(global_state.player.pos.y));
}

void update_motion_syscall(vm_state_t* state)
{
    var_t ptr = pop_stack(state);
    if (VAR_TYPE(ptr) != PTR)
        return;

    memory_region_t* region = &state->mem_regions[VAR_OBJECT(ptr)];
    if (region->size < 13)
        return;

    motion_data_t motion = {0};
    motion.max_rot = motion.max_accel = motion.max_speed = motion.max_angular = +INFINITY;

    motion.relative_x = VAR_VAL_FLT(region->base[0]);
    motion.relative_y = VAR_VAL_FLT(region->base[1]);
    motion.velocity = VAR_VAL_FLT(region->base[2]);
    motion.acceleration = VAR_VAL_FLT(region->base[3]);
    motion.direction_angle = VAR_VAL_FLT(region->base[4]);
    motion.angular_velocity = VAR_VAL_FLT(region->base[5]);
    motion.rotation = VAR_VAL_FLT(region->base[6]);
    motion.rotational_speed = VAR_VAL_FLT(region->base[7]);

    update_motion(frame_time, &motion);

    region->base[0] = MK_FLT(motion.relative_x);
    region->base[1] = MK_FLT(motion.relative_y);
    region->base[2] = MK_FLT(motion.velocity);
    region->base[3] = MK_FLT(motion.acceleration);
    region->base[4] = MK_FLT(motion.direction_angle);
    region->base[5] = MK_FLT(motion.angular_velocity);
    region->base[6] = MK_FLT(motion.rotation);
    region->base[7] = MK_FLT(motion.rotational_speed);
}

void get_frame_syscall(vm_state_t* state)
{
    push_stack(state, MK_VAR(global_state.current_frame, INT));
}

void create_bullet_syscall(vm_state_t* state)
{
    var_t type = pop_stack(state);
    var_t ptr = pop_stack(state);
    if (VAR_TYPE(type) != INT)
        goto error;
    if (VAR_TYPE(ptr) != PTR)
        goto error;

    memory_region_t* region = &state->mem_regions[VAR_OBJECT(ptr)];
    if (region->size < 13)
        goto error;

    bullet_t bullet = {0};
    if (VAR_VAL(type) == 0)
        bullet.type = Circle;
    else if (VAR_VAL(type) == 1)
        bullet.type = Rect;
    else // default
        bullet.type = Circle;
    bullet.sprite = sprite;
    bullet.motion.relative_x = VAR_VAL_FLT(region->base[0]);
    bullet.motion.relative_y = VAR_VAL_FLT(region->base[1]);
    bullet.motion.velocity = VAR_VAL_FLT(region->base[2]);
    bullet.motion.acceleration = VAR_VAL_FLT(region->base[3]);
    bullet.motion.direction_angle = VAR_VAL_FLT(region->base[4]);
    bullet.motion.angular_velocity = VAR_VAL_FLT(region->base[5]);
    bullet.motion.rotation = VAR_VAL_FLT(region->base[6]);
    bullet.motion.rotational_speed = VAR_VAL_FLT(region->base[7]);
    if (bullet.type == Circle)
    {
        circle_info_t info = {0};
        info.radius = 16.f;
        info.hitbox_radius = 5.f;
        register_bullet(bullet, &info);
    }
    else if (bullet.type == Rect)
    {
        bullet.sprite = shard_sprite;
        rect_info_t info = {0};
        info.width = 16.f;
        info.height = 18.f;
        info.hitbox_width = 5.f;
        info.hitbox_height = 10.f;
        register_bullet(bullet, &info);
    }

    push_stack(state, MK_VAR(0, INT)); // supposed to be the id
    return;

error:
    push_stack(state, MK_VAR(-1, INT)); // supposed to be the id
}

void load_music_syscall(vm_state_t* state)
{
    var_t path = pop_stack(state);
    if (VAR_TYPE(path) != STR)
    {
        push_stack(state, MK_VAR(-1, INT));
        return;
    }

    char path_buf[1024];
    vm_read_str(state, path, path_buf, 1024);

    music_id_t music = load_music(path_buf);
    push_stack(state, MK_VAR(music, INT));
}

void play_music_syscall(vm_state_t* state)
{
    var_t music_var = pop_stack(state);
    if (VAR_TYPE(music_var) != INT)
        return;

    if (current_bgm != INVALID_MUSIC_ID)
        stop_music(current_bgm);

    music_id_t music = VAR_VAL(music_var);
    current_bgm = music;
    play_music(music);
}

void load_spritesheet_syscall(vm_state_t* state)
{
    var_t key = pop_stack(state);
    var_t path = pop_stack(state);
    if (VAR_TYPE(path) != STR || VAR_TYPE(key) != STR)
    {
        push_stack(state, MK_VAR(-1, INT));
        return;
    }

    char path_buf[1024];
    vm_read_str(state, path, path_buf, 1024);
    char key_buf[128];
    vm_read_str(state, key, key_buf, 128);

    spritesheet_id_t id = load_spritesheet(path_buf, key_buf);
    push_stack(state, MK_VAR(id, INT));
}

void load_sprite_id_syscall(vm_state_t* state)
{
    var_t rect_ptr = pop_stack(state);
    var_t sheet_id = pop_stack(state);
    if (VAR_TYPE(sheet_id) != INT)
        goto error;
    if (VAR_TYPE(rect_ptr) != PTR)
        goto error;

    memory_region_t* region = &state->mem_regions[VAR_OBJECT(rect_ptr)];
    if (region->size < 4)
        goto error;

    rect_t rect;
    rect.x = VAR_VAL_FLT(region->base[0]);
    rect.y = VAR_VAL_FLT(region->base[1]);
    rect.w = VAR_VAL_FLT(region->base[2]);
    rect.h = VAR_VAL_FLT(region->base[3]);

    sprite_frame_id_t id = load_sprite_frame(sheet_id, rect);
    push_stack(state, MK_VAR(id, INT));

    return;
error:
    push_stack(state, MK_VAR(-1, INT));
}

void draw_sprite_syscall(vm_state_t* state)
{
    var_t angle  = pop_stack(state);
    var_t size_y = pop_stack(state);
    var_t size_x = pop_stack(state);
    var_t pos_y = pop_stack(state);
    var_t pos_x = pop_stack(state);
    var_t sheet_id = pop_stack(state);
    var_t zorder = pop_stack(state);
    if (VAR_TYPE(sheet_id) != INT || VAR_TYPE(zorder) != INT)
        return;
    if (VAR_TYPE(pos_x) != FLOAT || VAR_TYPE(pos_y) != FLOAT)
        return;
    if (VAR_TYPE(size_x) != FLOAT || VAR_TYPE(size_y) != FLOAT)
        return;
    if (VAR_TYPE(angle) != FLOAT)
        return;

    sprite_frame_id_t id = VAR_VAL(sheet_id);
    vector2d_t pos, size;
    pos.x = VAR_VAL_FLT(pos_x); pos.y = VAR_VAL_FLT(pos_y);
    size.x = VAR_VAL_FLT(size_x); size.y = VAR_VAL_FLT(size_y);

    vm_drawlist_register_sprite(id, pos, size, VAR_VAL_FLT(angle), VAR_VAL(zorder));
}

void draw_animated_sprite_syscall(vm_state_t* state)
{
    var_t frames_ptr  = pop_stack(state);
    var_t total_frames = pop_stack(state);
    var_t var_start_frame = pop_stack(state);
    var_t var_frame_count = pop_stack(state);
    var_t angle  = pop_stack(state);
    var_t size_y = pop_stack(state);
    var_t size_x = pop_stack(state);
    var_t pos_y = pop_stack(state);
    var_t pos_x = pop_stack(state);
    var_t var_id = pop_stack(state);
    var_t zorder = pop_stack(state);
    if (VAR_TYPE(var_id) != INT || VAR_TYPE(zorder) != INT || VAR_TYPE(total_frames) != INT)
        return;
    if (VAR_TYPE(var_start_frame) != INT || VAR_TYPE(var_frame_count) != INT)
        return;
    if (VAR_TYPE(frames_ptr) != PTR)
        return;
    if (VAR_TYPE(pos_x) != FLOAT || VAR_TYPE(pos_y) != FLOAT)
        return;
    if (VAR_TYPE(size_x) != FLOAT || VAR_TYPE(size_y) != FLOAT)
        return;
    if (VAR_TYPE(angle) != FLOAT)
        return;

    memory_region_t* region = &state->mem_regions[VAR_OBJECT(frames_ptr)];
    if (region->size < VAR_VAL(var_frame_count))
        return;
    if (VAR_VAL(var_frame_count) >= MAX_ANIM_FRAMES)
        return;

    animation_t anim;
    anim.sheet = VAR_VAL(var_id);
    anim.anim_frame_count = VAR_VAL(var_frame_count);
    anim.start_frame = VAR_VAL(var_start_frame);
    anim.total_frametime = VAR_VAL(total_frames);
    for (unsigned i = 0; i < VAR_VAL(var_frame_count); ++i)
    {
        const size_t frame_size = 5;  // rect_t + int (anim_duration)

        var_t* frame_ptr = &region->base[i*frame_size];

        anim.frames[i].spritesheet_rect.x = VAR_VAL_FLT(frame_ptr[0]);
        anim.frames[i].spritesheet_rect.y = VAR_VAL_FLT(frame_ptr[1]);
        anim.frames[i].spritesheet_rect.w = VAR_VAL_FLT(frame_ptr[2]);
        anim.frames[i].spritesheet_rect.h = VAR_VAL_FLT(frame_ptr[3]);
        anim.frames[i].frame_duration = VAR_VAL(frame_ptr[4]);
    }

    vector2d_t pos, size;
    pos.x = VAR_VAL_FLT(pos_x); pos.y = VAR_VAL_FLT(pos_y);
    size.x = VAR_VAL_FLT(size_x); size.y = VAR_VAL_FLT(size_y);

    vm_drawlist_register_anim(&anim, pos, size, VAR_VAL_FLT(angle), VAR_VAL(zorder));
}

void easing_syscall(vm_state_t* vm)
{
    var_t easing_func = pop_stack(vm);
    var_t float_in = pop_stack(vm);
    if (VAR_TYPE(easing_func) != INT || VAR_TYPE(float_in) != FLOAT)
    {
        float val = 0.0f;
        push_stack(vm, MK_FLT(val));
        return;
    }

    easing_functions func = (easing_functions)VAR_VAL(easing_func);
    easing_function_t ptr = getEasingFunction(func);

    float val = ptr(VAR_VAL_FLT(float_in));
    push_stack(vm, MK_FLT(val));
    return;
}

void get_obj3d_syscall(vm_state_t* vm)
{
    uint16_t region_id = alloc_memory_region(vm, sizeof(var_t)*(3*3 + 1)); // 3 vec3 + 1 tex_id

    var_t id_var = pop_stack(vm);
    if (VAR_TYPE(id_var) != INT)
    {
        push_stack(vm, MK_PTR(region_id));
        return;
    }
    obj3d_t* ref = get_obj3d_ref(VAR_VAL(id_var));
    if (ref == NULL)
    {
        push_stack(vm, MK_PTR(region_id));
        return;
    }
    // copy data
    vm->mem_regions[region_id].base[0] = MK_FLT(ref->pos.x);
    vm->mem_regions[region_id].base[1] = MK_FLT(ref->pos.y);
    vm->mem_regions[region_id].base[2] = MK_FLT(ref->pos.z);

    vm->mem_regions[region_id].base[3] = MK_FLT(ref->orientation.x);
    vm->mem_regions[region_id].base[4] = MK_FLT(ref->orientation.y);
    vm->mem_regions[region_id].base[5] = MK_FLT(ref->orientation.z);

    vm->mem_regions[region_id].base[6] = MK_FLT(ref->scale.x);
    vm->mem_regions[region_id].base[7] = MK_FLT(ref->scale.y);
    vm->mem_regions[region_id].base[8] = MK_FLT(ref->scale.z);

    vm->mem_regions[region_id].base[9] = MK_VAR(ref->texture, INT);

    push_stack(vm, MK_PTR(region_id));
}

void update_obj3d_syscall(vm_state_t* vm)
{
    var_t obj_ptr = pop_stack(vm);
    var_t obj_id  = pop_stack(vm);
    if (VAR_TYPE(obj_id) != INT || VAR_TYPE(obj_ptr) != PTR)
        return;
    if (VAR_OBJECT(obj_ptr) >= MAX_MEMORY_REGIONS ||
        (VAR_OFFSET(obj_ptr) + vm->mem_regions[VAR_OBJECT(obj_ptr)].size) < (3*3+1))
        return;

    obj3d_t* ref = get_obj3d_ref(VAR_VAL(obj_id));
    if (ref == NULL)
        return;

    ref->pos.x = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+0]);
    ref->pos.y = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+1]);
    ref->pos.z = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+2]);
    ref->orientation.x = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+3]);
    ref->orientation.y = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+4]);
    ref->orientation.z = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+5]);
    ref->scale.x = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+6]);
    ref->scale.y = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+7]);
    ref->scale.z = VAR_VAL_FLT(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+8]);

    ref->texture = VAR_VAL(vm->mem_regions[VAR_OBJECT(obj_ptr)].base[VAR_OFFSET(obj_ptr)+9]);
}

void create_obj3d_syscall(vm_state_t* vm)
{
    var_t path = pop_stack(vm);
    if (VAR_TYPE(path) != STR)
    {
        push_stack(vm, MK_VAR(-1, INT));
        return;
    }

    char path_buf[1024];
    vm_read_str(vm, path, path_buf, 1024);

    obj3d_id_t id = load_obj3d(path_buf);
    push_stack(vm, MK_VAR(id, INT));
}

void load_texture_syscall(vm_state_t* vm)
{
    var_t key = pop_stack(vm);
    var_t path = pop_stack(vm);
    if (VAR_TYPE(path) != STR || VAR_TYPE(key) != STR)
    {
        push_stack(vm, MK_VAR(-1, INT));
        return;
    }

    char path_buf[1024];
    vm_read_str(vm, path, path_buf, 1024);
    char key_buf[128];
    vm_read_str(vm, key, key_buf, 128);

    texture_t tex = load_texture(path_buf);
    texture_id_t id = register_texture(tex, key_buf);
    push_stack(vm, MK_VAR(id, INT));
}

void load_emitter_syscall(vm_state_t* vm)
{
    var_t path = pop_stack(vm);
    if (VAR_TYPE(path) != STR)
    {
        push_stack(vm, MK_VAR(-1, INT));
        return;
    }

    char path_buf[1024];
    vm_read_str(vm, path, path_buf, 1024);

    emitter_t* emitter = load_emitter_from_file(path_buf);

    push_stack(vm, MK_VAR(0, INT)); // TODO : replace with emitter ID
}

void register_entity_syscall(vm_state_t* vm)
{
    var_t name = pop_stack(vm);
    if (VAR_TYPE(name) != STR)
        return;

    char name_buf[MAX_ENTITY_NAME_LEN];
    vm_read_str(vm, name, name_buf, MAX_ENTITY_NAME_LEN);

    register_entity(name_buf);

    motion_data_t motion = {0};
    entity_value_t val; val.motion = motion;
    set_entity_value(name_buf, "motion", &val); // init the 'motion' attribute
}

void update_entity_syscall(vm_state_t* vm)
{
    var_t pos_y = pop_stack(vm);
    var_t pos_x = pop_stack(vm);
    var_t name = pop_stack(vm);
    if (VAR_TYPE(pos_x) != FLOAT || VAR_TYPE(pos_y) != FLOAT)
        return;
    if (VAR_TYPE(name) != STR)
        return;

    char name_buf[MAX_ENTITY_NAME_LEN];
    vm_read_str(vm, name, name_buf, MAX_ENTITY_NAME_LEN);

    motion_data_t motion = {0};
    motion.relative_x = VAR_VAL_FLT(pos_x);
    motion.relative_y = VAR_VAL_FLT(pos_y);
    entity_value_t val; val.motion = motion;
    set_entity_value(name_buf, "motion", &val);
}

void init_scripting_syscalls(vm_state_t *vm)
{
    register_syscall(vm, 0x80, create_bullet_syscall);
    register_syscall(vm, 0x81, get_frame_syscall);
    register_syscall(vm, 0x82, player_pos_syscall);
    register_syscall(vm, 0x83, update_motion_syscall);
    register_syscall(vm, 0x84, load_music_syscall);
    register_syscall(vm, 0x85, play_music_syscall);
    register_syscall(vm, 0x86, load_spritesheet_syscall);
    register_syscall(vm, 0x87, load_sprite_id_syscall);
    register_syscall(vm, 0x88, draw_sprite_syscall);
    register_syscall(vm, 0x89, draw_animated_sprite_syscall);
    register_syscall(vm, 0x8A, easing_syscall);
    register_syscall(vm, 0x8B, get_obj3d_syscall);
    register_syscall(vm, 0x8C, update_obj3d_syscall);
    register_syscall(vm, 0x8D, create_obj3d_syscall);
    register_syscall(vm, 0x8E, load_texture_syscall);
    register_syscall(vm, 0x8F, load_emitter_syscall);
    register_syscall(vm, 0x90, register_entity_syscall);
    register_syscall(vm, 0x91, update_entity_syscall);
}
