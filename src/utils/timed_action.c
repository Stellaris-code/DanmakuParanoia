#include "timed_action.h"

#include <string.h>

#include "sys/log.h"
#include "sys/cleanup.h"

typedef struct action_entry_t
{
    float remaining_time;
    void (*callback)(void*);
    void* arg;
} action_entry_t;

static action_entry_t entries[MAX_TIMED_ACTIONS];

void schedule_action(float time, void (*callback)(void *), void *arg)
{
    for (unsigned i = 0; i < MAX_TIMED_ACTIONS; ++i)
    {
        if (!entries[i].callback) // unused
        {
            entries[i].remaining_time = time;
            entries[i].callback = callback;
            entries[i].arg = arg;
            return;
        }
    }

    trace_log(LOG_WARNING, "Could not allocate timed action");
}

void timed_actions_update(float dt)
{
    for (unsigned i = 0; i < MAX_TIMED_ACTIONS; ++i)
    {
        if (!entries[i].callback)
            continue;

        if ((entries[i].remaining_time -= dt) <= 0)
        {
            entries[i].callback(entries[i].arg);
            entries[i].callback = 0; // reset
        }
    }
}

static void cleanup_timed_actions()
{
    memset(entries, 0, sizeof(entries));
}

void init_timed_actions()
{
    register_cleanup(cleanup_timed_actions, GamestateEnd);
}
