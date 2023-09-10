#ifndef PARTICLES_H
#define PARTICLES_H

#include "gameplay/motion.h"

#include "color.h"
#include "blend_mode.h"
#include "resources/texture_handler.h"
#include "math/easing.h"

#define MAX_PARTICLE_ARCHETYPES 4
#define MAX_ALIVE_PARTICLES 4096
#define MAX_EMITTERS 1024

typedef enum emitter_archetype_strategy
{
    LOOP,
    RANDOM
} emitter_archetype_strategy;

typedef struct particle_t
{
    motion_data_t motion;
    float lifetime;

    color_t initial_color;
    color_t final_color;

    vector2d_t initial_scale;
    vector2d_t final_scale;

    easing_function_enum scale_easing;
    easing_function_enum color_easing;

    sprite_frame_id_t spriteframe;
} particle_t;

typedef struct emitter_t
{
    motion_data_t motion;

    float lifetime;

    unsigned int archetype_count;
    emitter_archetype_strategy archetype_strategy;

    spritesheet_id_t particles_sheet;
    blend_mode blend_mode;

    int zorder;
    float frequency; // number of bursts per second
    unsigned int burst_size; // number of particles created per burst

    motion_data_t motion_randomness; // factors to add randomness to the particles' motion

    particle_t particle_archetypes[MAX_PARTICLE_ARCHETYPES];
} emitter_t;

extern unsigned int particle_count;

void init_particle_system();
void update_particle_system(float dt);
emitter_t* create_emitter(); // returns a pointer to fill with emitter data
void draw_particles();
void clear_particles();
emitter_t *load_emitter_from_file(const char* file);

#endif // PARTICLES_H
