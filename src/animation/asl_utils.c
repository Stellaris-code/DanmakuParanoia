#include "anim_sequence.h"

#include <string.h>
#include <assert.h>

#include "math/easing.h"

const char *asl_string(const asl_program_t* program, str_entry_t idx)
{
    if (idx >= program->string_table.size)
        return NULL;
    uint32_t off = program->string_table.ptr[idx];
    assert(off <= program->string_data_idx); // the offset can't be greater than the latest position within the string data

    return program->string_data + off;
}

asl_argument_t* get_argument_ptr(const asl_program_t* program, const char* arg_name, int default_pos, asl_timed_action_t* action)
{
    asl_argument_t* arg_list = action->args;
    unsigned arg_count = action->arg_count;

    for (unsigned i = 0; i < arg_count; ++i)
    {
        if (arg_list[i].name && strcmp(arg_name, asl_string(program, arg_list[i].name)) == 0)
        {
            return &arg_list[i];
        }
    }

    if (default_pos < 0 || (unsigned)default_pos >= arg_count)
        return NULL;

    return &arg_list[default_pos];
}

float get_argument_flt(const asl_timed_action_t* asl_action, asl_exec_state_t* state, asl_argument_t* ptr, asl_flt_arg_role_t role)
{
    if (!ptr || ptr->type == ASLArgString)
        return 0.0f;

    float end_time = asl_action->start_time + asl_action->duration;

    float t = (state->elapsed_time - asl_action->start_time) / (end_time - asl_action->start_time);
    if (t > 1.0f) t = 1.0f;

    easing_function_t easing_func = getEasingFunction(ptr->interp);

    // Compute the value at current time of dynamic values
    if (ptr->dyn_tag == Percentage)
    {
        if (ptr->type == ASLArgLength)
        {
            if (role == Width || role == PosX)
                return easing_func(t) * ptr->length_pct * state->env_info.screen_width;
            if (role == Height || role == PosY)
                return easing_func(t) * ptr->length_pct * state->env_info.screen_height;
        }
    }
    // Static Value
    {
        return easing_func(t) * ptr->angle_rad; // or the other float arguments, as they're an union
    }
}

float get_argument_flt_nointerp(asl_argument_t* ptr)
{
    if (!ptr || ptr->type == ASLArgString)
        return 0.0;

    return ptr->angle_rad; // or the other float arguments, as they're an union
}
