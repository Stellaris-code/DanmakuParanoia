#include "cleanup.h"

#include <assert.h>

#include "sys/log.h"

#define MAX_CLEANUP_FUNCTIONS 4096

static cleanup_callback cleanup_functions[MAX_CLEANUP_FUNCTIONS];
static int cleanup_function_count;

void register_cleanup(cleanup_callback callback)
{
    if (cleanup_function_count >= MAX_CLEANUP_FUNCTIONS)
    {
        trace_log(LOG_ERROR, "Couldn't register cleanup function : out of slots");
        return;
    }

    cleanup_functions[cleanup_function_count] = callback;
    ++cleanup_function_count;
}

void do_cleanup()
{
    trace_log(LOG_INFO, "Calling %d cleanup functions", cleanup_function_count);
    for (int i = 0; i < cleanup_function_count; ++i)
    {
        assert(cleanup_functions[i]);
        cleanup_functions[i]();
    }
}
