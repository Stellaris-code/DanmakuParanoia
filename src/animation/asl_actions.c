#include "anim_sequence.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define COMPLETED(action, state) ((action)->start_time + (action)->duration <= (state)->elapsed_time)

int asl_exec_wait(asl_timed_action_t* asl_action, asl_exec_state_t* state, float dt);
int asl_print(asl_timed_action_t* asl_action, asl_exec_state_t* state, float dt);
int asl_rotate(asl_timed_action_t* asl_action, asl_exec_state_t* state, float dt);

// returns 1 if completed
timed_action_handler_t timed_action_list[] = {
    {"wait", &asl_exec_wait},
    {"print", &asl_print},
    {"rotate", &asl_rotate},
    {NULL, NULL}
};

timed_action_handler_t* find_timed_action(const char* name)
{
    timed_action_handler_t* timed_action_ptr = timed_action_list;

    while (timed_action_ptr->handler) // sentinel value is NULL
    {
        if (strcmp(name, timed_action_ptr->action_name) == 0)
            return timed_action_ptr;
        ++timed_action_ptr;
    }
    return NULL;
}

int asl_exec_wait(asl_timed_action_t* asl_action, asl_exec_state_t* state, float dt)
{
    asl_argument_t* arg_ptr = get_argument_ptr(state->program, "time", 0, asl_action);
    if (arg_ptr) // if there's an argument instead of a 'for/in' duration, prioritize the argument
    {
        asl_action->duration = get_argument_flt_nointerp(arg_ptr); // interpolation functions here are irrelevant, ignore
    }

    return COMPLETED(asl_action, state);
}

int asl_print(asl_timed_action_t* asl_action, asl_exec_state_t* state, float dt)
{
    asl_argument_t* arg_ptr = get_argument_ptr(state->program, "", 0, asl_action);
    if (arg_ptr && arg_ptr->type == ASLArgString) // if there's an argument instead of a 'for/in' duration, prioritize the argument
        printf("%s\n", asl_string(state->program, arg_ptr->string));

    return 1;
}

int asl_rotate(asl_timed_action_t* asl_action, asl_exec_state_t* state, float dt)
{
    asl_argument_t* arg = get_argument_ptr(state->program, "angle", 0, asl_action);
    if (!arg || arg->type != ASLArgAngle)
    {
        // TODO
        // ...
        assert(0);
        return 1;
    }

    if (asl_action->start_time < 0)
        asl_action->start_time = state->elapsed_time;

    float angle = get_argument_flt(asl_action, state, arg, Angle);
    printf("Angle: %f (%f) (%f vs %f)\n", angle/3.1415f*180.f, arg->angle_rad/3.1415f*180.f,
           state->elapsed_time, asl_action->start_time + asl_action->duration);

    return COMPLETED(asl_action, state);
}

