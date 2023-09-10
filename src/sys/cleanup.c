#include "cleanup.h"

#include <assert.h>

#include "sys/log.h"

#define MAX_CLEANUP_FUNCTIONS 4096

typedef struct cleanup_table_entry_t
{
    cleanup_callback callback;
    cleanup_event event;
} cleanup_table_entry_t;

static cleanup_table_entry_t cleanup_functions[MAX_CLEANUP_FUNCTIONS];
static int cleanup_function_count;

void register_cleanup(cleanup_callback callback, cleanup_event event)
{
    if (cleanup_function_count >= MAX_CLEANUP_FUNCTIONS)
    {
        trace_log(LOG_ERROR, "Couldn't register cleanup function : out of slots");
        return;
    }

    cleanup_functions[cleanup_function_count].callback = callback;
    cleanup_functions[cleanup_function_count].event = event;
    ++cleanup_function_count;
}

void do_cleanup(cleanup_event event)
{
    trace_log(LOG_INFO, "Calling %d cleanup functions for event %d", cleanup_function_count, event);
    for (int i = 0; i < cleanup_function_count; ++i)
    {
        assert(cleanup_functions[i].callback);
        if (cleanup_functions[i].event <= event)
            cleanup_functions[i].callback();
    }
}
