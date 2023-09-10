#include "anim_sequence.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "sys/alloc.h"

void parse_program(const char* source, asl_program_t* program);

_Noreturn static void error()
{
    abort();
}

// returns the index of the created coroutine if successful, -1 otherwise
int asl_create_coroutine(asl_exec_state_t* state, asl_base_t* action)
{
    int index = -1;
    for (int i = 0; i < MAX_COROUTINES; ++i)
    {
        if (state->coroutines[i].action == NULL)
        {
            index = i;
            break;
        }
    }
    // all coroutines are allocated
    if (index == -1)
    {
        fprintf(stderr, "out of ASL coroutines\n");
        return -1;
    }

    state->coroutines[index].action = action;
    state->coroutines[index].start_time = state->elapsed_time;
    state->coroutines[index].dependencies = 0;

    //printf("Creating coroutine %d\n", index);

    return index;
}

void asl_init_exec(asl_exec_state_t* state, asl_program_t* program)
{
    asl_base_t* first_action = program->action_list;

    state->program = program;
    state->elapsed_time = 0.f;

    memset(&state->coroutines, 0, sizeof(asl_coroutine_t) * MAX_COROUTINES);
    asl_create_coroutine(state, first_action);
}

void asl_cleanup(asl_program_t* program)
{
    danpa_free(program->string_data);
    DYNARRAY_FREE(program->macros);
    DYNARRAY_FREE(program->string_table);
    free_hash_table(&program->string_hash_table);

    for (int i = 0; i < program->allocated_action_list.size; ++i)
    {
        asl_base_t* action = program->allocated_action_list.ptr[i];
        // Call the destructor for this type of action
        if (action_destructors[action->type])
        {
            action_destructors[action->type](action);
        }
        danpa_free(action);
    }
    DYNARRAY_FREE(program->allocated_action_list);
}

asl_base_t* asl_handle_parallel(asl_parallel_t* parallel, asl_exec_state_t* state, float dt);
asl_base_t* asl_exec_for(asl_for_t* asl_for, asl_exec_state_t* state, float dt);
asl_base_t* asl_exec_play(asl_play_t* asl_play, asl_exec_state_t* state, float dt);
// returns either the same action if it's not completed yet, the next action to execute, or NULL if it reached the end of the action chain
asl_base_t* asl_exec_action(asl_base_t* action, asl_exec_state_t* state, float dt)
{
    do
    {
        switch (action->type)
        {
            case ASL_define:
            {
                asl_define_t* def = (asl_define_t*)action;
                return def->actual_next_action;
                break;
            }

            case ASL_parallel:
            {
                asl_parallel_t* parallel = (asl_parallel_t*)action;

                return asl_handle_parallel(parallel, state, dt);
                break;
            }

            case ASL_for:
            {
                asl_for_t* asl_for = (asl_for_t*)action;

                return asl_exec_for(asl_for, state, dt);
            }

            case ASL_play:
            {
                asl_play_t* asl_play = (asl_play_t*)action;

                return asl_exec_play(asl_play, state, dt);
                break;
            }

            case ASL_timed_action:
            {
                asl_timed_action_t* timed_action = (asl_timed_action_t*)action;
                //printf("Handling '%s'\n", timed_action->action_name);

                timed_action_handler_t* timed_action_ptr = find_timed_action(asl_string(state->program, timed_action->action_name));

                if (!timed_action_ptr)
                {
                    // unhandled action
                    //error();
                    printf("Unhandled action '%s'\n", asl_string(state->program, timed_action->action_name));
                    // skip the action
                    return action->next;
                }

                // action was not run yet, initialize its starting time
                if (timed_action->start_time < 0)
                    timed_action->start_time = state->elapsed_time;
                int completed = timed_action_ptr->handler(timed_action, state, dt);
                if (completed)
                {
                    timed_action->start_time = -1; // reset start time
                    return action->next;
                }
                else
                    return action;

                break;
            }

            default:
                assert(0 && "unhandled asl action type");
        }
    } while (0);

    // here be dragons
    return NULL; // should never reach here, but who knows
}

// returns 0 if the end isn't reached yet, 1 if so
asl_base_t* asl_handle_parallel(asl_parallel_t* parallel, asl_exec_state_t* state, float dt)
{
    (void)dt;

    assert(state->coroutines[state->current_coroutine].dependencies == 0);

    // create a coroutine for each parallel action, and update the current coroutine so that it can only resume execution if all the slave coroutines are done

    for (int entry_idx = 0; entry_idx < parallel->entry_list.size; ++entry_idx)
    {
        parallel_entry_t* entry = &parallel->entry_list.ptr[entry_idx];

        int coroutine_id = asl_create_coroutine(state, entry->action_list);
        if (coroutine_id < 0)
            break;

        state->coroutines[coroutine_id].end_ptr = entry->end_ptr;

        state->coroutines[state->current_coroutine].dependencies |= (1 << (uint8_t)coroutine_id);
    }

    return parallel->base.next;
}

asl_base_t* asl_exec_play(asl_play_t* asl_play, asl_exec_state_t* state, float dt)
{
    printf("Executing macro '%s'\n", asl_string(state->program, asl_play->macro_name));

    asl_macro_entry_t* macro_ptr = NULL;
    int macro_found = 0;
    for (int macro_idx = 0; macro_idx < state->program->macros.size; ++macro_idx)
    {
        macro_ptr = &state->program->macros.ptr[macro_idx];
        // Same string entry number == same string
        if (asl_play->macro_name == macro_ptr->name)
        {
            macro_found = 1;
            break;
        }
    }

    if (!macro_found)
    {
        printf("No such macro '%s' found\n", asl_string(state->program, asl_play->macro_name));
        //error(); // not found;
        return asl_play->base.next; // skip
    }

    // create a coroutine executing the macro and ending at the macro's last instruction
    int macro_coroutine_id = asl_create_coroutine(state, macro_ptr->action_list);

    state->coroutines[macro_coroutine_id].end_ptr = NULL;
    state->coroutines[state->current_coroutine].dependencies |= (1 << (uint8_t)macro_coroutine_id);

    return asl_play->base.next;
}

asl_base_t* asl_exec_for(asl_for_t* asl_for, asl_exec_state_t* state, float dt)
{
    (void)state; (void)dt;
    // loop completed, return next action
    if (asl_for->i >= asl_for->loop_count)
    {
        asl_for->i = 0;
        return asl_for->base.next;
    }
    // execute loop body
    else
    {
        ++asl_for->i;
        return asl_for->action_list;
    }
}

void asl_exit_coroutine(asl_exec_state_t* state, unsigned coroutine_idx)
{
    assert(coroutine_idx < MAX_COROUTINES);
    asl_coroutine_t* entry = &state->coroutines[coroutine_idx];

    entry->action = NULL; // clear the entry
    uint64_t dependency_clear_mask = ~(1 << coroutine_idx);

    // clear the dependancy mask of every other coroutine
    for (unsigned i = 0; i < MAX_COROUTINES; ++i)
        state->coroutines[i].dependencies &= dependency_clear_mask;
}

// returns 0 if the end isn't reached yet, 1 if so
int asl_step_run(asl_exec_state_t* state, float dt)
{
    assert(state);

    state->elapsed_time += dt;

    int end_reached = 1;

reloop:
    for (unsigned i = 0; i < MAX_COROUTINES; ++i)
    {
        state->current_coroutine = i;

        while (state->coroutines[i].action && state->coroutines[i].dependencies == 0)
        {
            asl_base_t* next_action = asl_exec_action(state->coroutines[i].action, state, dt);
            const int completed = next_action != state->coroutines[i].action;

            // we reached the end of this coroutine's actions
            if (completed && state->coroutines[i].dependencies == 0 && next_action == state->coroutines[i].end_ptr)
            {
                asl_exit_coroutine(state, i);
                // dependencies might have been cleared, loop back through all coroutines as some might have been freed from their 'waiting' state
                goto reloop;
            }
            // action was completed, try the next one
            else if (completed)
            {
                state->coroutines[i].action = next_action;
                continue;
            }
            // action is not yet completed, skip to the other coroutines
            else
            {
                end_reached = 0;
                break;
            }
        }

        // this task isn't running but is waiting on something, report that we aren't done yet
        if (state->coroutines[i].dependencies != 0)
            end_reached = 0;
    }

    return end_reached;
}

void destruct_parallel(asl_base_t* base)
{
    asl_parallel_t* parallel = (asl_parallel_t*)base;
    DYNARRAY_FREE(parallel->entry_list);
}

action_destructor action_destructors[ASL_TYPE_MAX] =
    {
        NULL, // define
        destruct_parallel, // parallel
        NULL, // seq
        NULL, // for
        NULL, // play
        NULL  // timed action
};

