/*
 * <32021 by Stellaris. Copying Art is an act of love. Love is not subject to law.
 */
#ifndef ENGINE_H
#define ENGINE_H

#define TARGET_FPS 60
#define FIXED_TIMESTEP (1.0f/TARGET_FPS)

#include "math/vector.h"

#include "state.h"

typedef struct engine_t
{
    state_t* state_stack[MAX_ACTIVE_STATES];
    unsigned active_states;

    ivector2d_t render_area_size;
    unsigned current_frame;

    float frame_time;
} engine_t;

void engine_init(engine_t* eng);
void engine_cleanup(engine_t* eng);

// 'eng' takes ownership of the state
state_t* engine_current_state(engine_t* eng);
void engine_push_state(engine_t* eng, state_t* state);
void engine_pop_state(engine_t* eng);
void engine_update(engine_t* eng, float dt);
void engine_run_loop(engine_t* eng);

extern engine_t engine;

#endif // ENGINE_H
