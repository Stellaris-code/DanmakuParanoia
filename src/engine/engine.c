/*
 * <32021 by Stellaris. Copying Art is an act of love. Love is not subject to law.
 */

#include "engine.h"

#include <assert.h>
#include <stdbool.h>

#include "sys/log.h"
#include "sys/cleanup.h"

#include "utils/timed_action.h"

engine_t engine;

void engine_init(engine_t *eng)
{
    eng->active_states = 0;
    eng->state_stack[0] = 0;
    eng->current_frame = 0;
}

void engine_cleanup(engine_t *eng)
{
    for (int i = 0; i < eng->active_states; ++i)
    {
        assert(eng->state_stack[i]);
        eng->state_stack[i]->exit(eng->state_stack[i]);
        eng->state_stack[i] = 0;
    }

    eng->active_states = 0;
}

void engine_push_state(engine_t *eng, state_t *state)
{
    assert(eng->active_states <= MAX_ACTIVE_STATES);

    if (eng->active_states == MAX_ACTIVE_STATES)
    {
        trace_log(LOG_FATAL, "Out of active state slots");
        return;
    }

    state->init(state);
    eng->state_stack[eng->active_states] = state;
    ++eng->active_states;
}

void engine_pop_state(engine_t *eng)
{
    assert(eng->active_states <= MAX_ACTIVE_STATES);

    if (eng->active_states == 0)
    {
        trace_log(LOG_FATAL, "Tried to pop an empty state stack");
        return;
    }

    --eng->active_states;
    eng->state_stack[eng->active_states]->exit(eng->state_stack[eng->active_states]);
    eng->state_stack[eng->active_states] = 0;

    do_cleanup(GamestateEnd);
}

void engine_update(engine_t* eng, float dt)
{
    timed_actions_update(dt);
}

void engine_run_loop(engine_t *eng)
{
    bool exit = false;
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        /*
        if (exit)
        {
            double bob = elapsed_time();
            game_exit();
            double diff = elapsed_time() - bob;
            printf("------  Exit time : %02.03f\n", diff*1000);

            bob = elapsed_time();
            game_init();
            diff = elapsed_time() - bob;
            printf("------  Init time : %02.03f\n", diff*1000);

        }
        */
        float dt = (1.0f/TARGET_FPS);
        engine.frame_time = dt;
        engine_update(eng, dt);

        state_t* current = engine_current_state(eng);
        current->update(current, dt);
        // current state has changed
        if (current != engine_current_state(eng))
        {
            if (eng->active_states == 0)
                exit = true;
            else
            {
                exit = false;
                // reloop
                continue;
            }
        }
        else
        {
            current->render(current);
            ++eng->current_frame;
            exit = false;
        }
    }
}

state_t *engine_current_state(engine_t *eng)
{
    assert(eng->active_states <= MAX_ACTIVE_STATES);
    return eng->state_stack[eng->active_states-1];
}
