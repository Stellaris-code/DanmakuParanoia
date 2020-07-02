
#include <stdio.h>
#include <stdarg.h>

#include <raylib.h>

#include "sys/log.h"

void trace_log(int log_level, const char *fmt, ...)
{
    char buffer[512];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 512, fmt, args);
    va_end(args);

    // ugly way to work around the fact that no 'vTraceLog' exists
    TraceLog(log_level, "%s", buffer);
}
