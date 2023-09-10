#include "particles.h"

#include <assert.h>
#include <string.h>

#include "drawlist.h"
#include "draw.h"
#include "zorder.h"

#include "utils/json.h"
#include "sys/log.h"
#include "sys/fs.h"
#include "sys/cleanup.h"
#include "sys/alloc.h"

typedef struct particle_instance_info_t
{
    int emitter;
    float time_left;
    vector2d_t scale;
    color_t color;
} particle_instance_info_t;

typedef struct emitter_instance_info_t
{
    bool just_spawned;
    float elapsed_time;
    unsigned int last_chosen_archetype;
    unsigned int particle_count;
} emitter_instance_info_t;

static particle_t particle_list[MAX_ALIVE_PARTICLES];
static particle_instance_info_t particle_info[MAX_ALIVE_PARTICLES];
static uint8_t particle_alive[MAX_ALIVE_PARTICLES];

static emitter_t emitter_list[MAX_EMITTERS];
static emitter_instance_info_t emitter_info[MAX_EMITTERS];
static uint8_t emitter_alive[MAX_EMITTERS];

unsigned int particle_count;

static void create_particle_instance(particle_t* archetype, int emitter)
{
    for (int i = 0; i < MAX_ALIVE_PARTICLES; ++i)
    {
        if (!particle_alive[i])
        {
            particle_list[i] = *archetype;

            particle_info[i].time_left = archetype->lifetime;
            particle_info[i].emitter = emitter;
            ++emitter_info[emitter].particle_count;
            particle_alive[i] = true;
            ++particle_count;
            return;
        }
    }

    // too many particles alive; drop it
}

static inline float random_offset(float spread)
{
    if (spread == 0.0)
        return 0.0;
    return randf_bounds(-spread/2.0f, spread/2.0f);
}

static void update_emitter(int i, float dt)
{
    emitter_info[i].elapsed_time += dt;
    if (emitter_list[i].lifetime != 0.0 && emitter_info[i].elapsed_time >= emitter_list[i].lifetime)
    {
        emitter_alive[i] = false;
        return;
    }

    update_motion(dt, &emitter_list[i].motion);

    bool spawn_burst = (int)((emitter_info[i].elapsed_time-dt)*emitter_list[i].frequency) !=
                       (int)(emitter_info[i].elapsed_time*emitter_list[i].frequency);
    if (emitter_info[i].just_spawned || spawn_burst)
    {
        for (unsigned int j = 0; j < emitter_list[i].burst_size; ++j)
        {
            particle_t* archetype;
            if (emitter_list[i].archetype_strategy == LOOP)
            {
                ++emitter_info[i].last_chosen_archetype;
                if (emitter_info[i].last_chosen_archetype >= emitter_list[i].archetype_count)
                    emitter_info[i].last_chosen_archetype = 0;
                archetype = &emitter_list[i].particle_archetypes[emitter_info[i].last_chosen_archetype];
            }
            else // RAND
            {
                int idx = rand()%emitter_list[i].archetype_count;
                archetype = &emitter_list[i].particle_archetypes[idx];
            }

            particle_t copy = *archetype;
            copy.motion.root = &emitter_list[i].motion;

#define ADD_RANDOMNESS(field) \
    copy.motion.field += random_offset(emitter_list[i].motion_randomness.field);

            ADD_RANDOMNESS(relative_x);
            ADD_RANDOMNESS(relative_y);
            ADD_RANDOMNESS(velocity);
            ADD_RANDOMNESS(acceleration);
            ADD_RANDOMNESS(direction_angle);
            ADD_RANDOMNESS(angular_velocity);
            ADD_RANDOMNESS(rotation);
            ADD_RANDOMNESS(rotational_speed);
            ADD_RANDOMNESS(rotational_acceleration);
            ADD_RANDOMNESS(dampening);

#undef ADD_RANDOMNESS
            create_particle_instance(&copy, i);
        }
    }

    emitter_info[i].just_spawned = false;
}

void update_particle_system(float dt)
{
    for (int i = 0; i < MAX_EMITTERS; ++i)
    {
        if (!emitter_alive[i])
            continue;

        update_emitter(i, dt);
    }

    for (int i = 0; i < MAX_ALIVE_PARTICLES; ++i)
    {
        if (!particle_alive[i])
            continue;

        particle_info[i].time_left -= dt;
        if (particle_info[i].time_left <= 0.0f)
        {
            particle_alive[i] = false;
            --emitter_info[particle_info[i].emitter].particle_count;
            --particle_count;
            continue;
        }

        update_motion(dt, &particle_list[i].motion);
        float progress = particle_info[i].time_left/particle_list[i].lifetime;
        float scale_easing = getEasingFunction(particle_list[i].scale_easing)(progress);
        float color_easing = getEasingFunction(particle_list[i].color_easing)(progress);
        particle_info[i].scale = vec2d_lerp(particle_list[i].final_scale, particle_list[i].initial_scale, scale_easing);
        particle_info[i].color = color_lerp(particle_list[i].final_color, particle_list[i].initial_color, color_easing);
    }
}

void cleanup_particle_system()
{
    memset(emitter_alive, 0, MAX_EMITTERS*sizeof(uint8_t));
    memset(particle_alive, 0, MAX_ALIVE_PARTICLES*sizeof(uint8_t));
    particle_count = 0;
}

void init_particle_system()
{
    register_cleanup(cleanup_particle_system, GamestateEnd);
}


// O(n²)... yeah, this is bad
static void draw_emitter_callback(void* emitter_ptr)
{

    emitter_t* emitter = emitter_ptr;
    int emitter_i = emitter - emitter_list;

    enable_blend_mode(emitter->blend_mode);

    int count = 0;

    for (int i = 0; i < MAX_ALIVE_PARTICLES; ++i)
    {
        if (!particle_alive[i])
            continue;
        if (particle_info[i].emitter != emitter_i)
            continue;

        ++count;
        particle_t* particle = &particle_list[i];
        sprite_frame_t frame = get_sprite_frame(particle->spriteframe);
        vector2d_t pos = absolute_pos(&particle->motion);
        vector2d_t size;
        size.x = frame.spritesheet_rect.w*particle_info[i].scale.x;
        size.y = frame.spritesheet_rect.h*particle_info[i].scale.y;

        draw_sprite(particle->spriteframe, pos, size, particle->motion.rotation, particle_info[i].color);
    }

    reset_blend_mode();
}

void draw_particles()
{
    for (int i = 0; i < MAX_EMITTERS; ++i)
    {
        // even if the emitter is dead, continue to draw its particle as long as some remain
        if (emitter_alive[i] && emitter_info[i].particle_count > 0)
        {
            register_draw_element(&emitter_list[i], draw_emitter_callback, emitter_list[i].zorder);
        }
    }
}

emitter_t *create_emitter()
{
    for (int i = 0; i < MAX_EMITTERS; ++i)
    {
        if (!emitter_alive[i] && emitter_info[i].particle_count == 0)
        {
            emitter_alive[i] = true;
            emitter_info[i].just_spawned = true;
            emitter_info[i].elapsed_time = 0.0f;
            emitter_info[i].last_chosen_archetype = 0;
            emitter_info[i].particle_count = 0;
            emitter_list[i] = (emitter_t){0};

            return &emitter_list[i];
        }
    }

    return &emitter_list[0]; // to be safe, return the first emitter of the list instead of a NULL to avoid potential SEGFAULTS due to me being stoopid
}

void clear_particles()
{
    memset(particle_alive, 0, MAX_ALIVE_PARTICLES);
    memset(emitter_alive, 0, MAX_EMITTERS);
    memset(emitter_info, 0, sizeof(emitter_instance_info_t)*MAX_EMITTERS);
    particle_count = 0;
}

#define KEYVAL_SIZE (4096)
#define STRING_SIZE (65536)

static json_key_value_t keyval_buf[KEYVAL_SIZE];
static char string_buf[STRING_SIZE];

// TODO : allow json file to reference other named entities for motion hierarchies
// TODO : add easing functions here
void load_particle_from_json(particle_t* particle, json_value_t* json, json_context_t* context)
{
    assert(json);
    if (json->type != JSON_OBJECT)
        return;

    const char* initial_color_str = json_string_or("rgba(1,1,1,1)", "/initial-color", json, context);
    const char* final_color_str = json_string_or("rgba(1,1,1,1)", "/final-color", json, context);
    particle->initial_color = str_to_color(initial_color_str);
    particle->final_color = str_to_color(final_color_str);
    particle->color_easing = strToEasing(json_string_or("cubic", "/color-easing", json, context));
    particle->scale_easing = strToEasing(json_string_or("cubic", "/scale-easing", json, context));
    particle->lifetime = json_float_or(1.0, "/lifetime", json, context);
    particle->initial_scale.x = json_float_or(1.0, "/initial-scale/0", json, context);
    particle->initial_scale.y = json_float_or(1.0, "/initial-scale/1", json, context);
    particle->final_scale.x = json_float_or(1.0, "/final-scale/0", json, context);
    particle->final_scale.y = json_float_or(1.0, "/final-scale/1", json, context);
    particle->spriteframe = (sprite_frame_id_t)-1;
    load_motion_from_json(&particle->motion, json_pointer("/motion", json, context), context);

    rect_t sprite_rect;
    sprite_rect.x = json_int_or(0, "/sprite/frame/0", json, context);
    sprite_rect.y = json_int_or(0, "/sprite/frame/1", json, context);
    sprite_rect.w = json_int_or(32, "/sprite/frame/2", json, context);
    sprite_rect.h = json_int_or(32, "/sprite/frame/3", json, context);
    const char* spritesheet = json_string_or("particles-sheet", "/sprite/sheet", json, context);
    particle->spriteframe = load_sprite_frame(get_spritesheet(spritesheet), sprite_rect);

    //printf("scale : %f %f\n", sprite_rect.x, sprite_rect.w);
}

emitter_t *load_emitter_from_file(const char *file)
{
    emitter_t* emitter = create_emitter();

    json_context_t context;
    context.string_buffer = string_buf; context.string_buffer_size = STRING_SIZE; context.string_realloc = NULL;
    context.key_val_buffer = keyval_buf, context.key_val_buffer_size = KEYVAL_SIZE; context.key_val_realloc = NULL;
    unsigned long json_len;
    char* json_data = read_file(file, &json_len);
    if (json_data == NULL)
    {
        trace_log(LOG_WARNING, "couldn't load particle file '%s'", file);
        return NULL;
    }
    json_result_t result;
    result = parse_json(json_data, json_len, &context);
    if (!result.accepted)
    {
        danpa_free(json_data);
        trace_log(LOG_WARNING, "couldn't load JSON particle file '%s' : '%s'", file, result.error.reason);
        return NULL;
    }

    emitter->lifetime  = json_float_or(0.0, "/lifetime", &result.value, &context);
    emitter->zorder    = json_int_or(BELOWALL_ZORDER, "/zorder", &result.value, &context);
    emitter->frequency = json_float_or(1.0, "/frequency", &result.value, &context);
    emitter->burst_size    = json_int_or(1, "/burst-size", &result.value, &context);
    const char* archetype_strat = json_string_or("loop", "/archetype-strategy", &result.value, &context);
    if (strcmp(archetype_strat, "loop") == 0)
        emitter->archetype_strategy = LOOP;
    else if (strcmp(archetype_strat, "random") == 0)
        emitter->archetype_strategy = RANDOM;
    else
    {
        trace_log(LOG_WARNING, "invalid archetype value (%s)", file);
        goto error;
    }
    const char* blendmode = json_string_or("alpha", "/blendmode", &result.value, &context);
    if (strcmp(blendmode, "alpha") == 0)
        emitter->blend_mode = BlendAlpha;
    else if (strcmp(blendmode, "additive") == 0)
        emitter->blend_mode = BlendAdditive;
    else if (strcmp(blendmode, "multiplied") == 0)
        emitter->blend_mode = BlendMultiplied;
    else
    {
        trace_log(LOG_WARNING, "invalid blendmode value (%s)", file);
        goto error;
    }
    load_motion_from_json(&emitter->motion, json_pointer("/motion", &result.value, &context), &context);
    load_motion_from_json(&emitter->motion_randomness, json_pointer("/motion-randomness", &result.value, &context), &context);


    emitter->archetype_count = 0;
    json_value_t* particle_types = json_pointer("/particles", &result.value, &context);
    if (particle_types && particle_types->type == JSON_ARRAY)
    {
        emitter->archetype_count = particle_types->object.entry_count;
        if (emitter->archetype_count >= MAX_PARTICLE_ARCHETYPES)
            emitter->archetype_count = MAX_PARTICLE_ARCHETYPES;

        unsigned int idx = particle_types->object.start_idx;
        for (unsigned i = 0; i < emitter->archetype_count; ++i)
        {
            load_particle_from_json(&emitter->particle_archetypes[i], &context.key_val_buffer[idx].value, &context);
            idx = context.key_val_buffer[idx].next_value_idx;
        }
    }

    printf("frequ : %d\n", emitter->archetype_count);

    printf("spawned!\n");

    danpa_free(json_data);
    return emitter;

error:
    danpa_free(json_data);
    return NULL;
}
