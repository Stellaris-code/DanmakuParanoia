#ifndef FILE_LOG_H
#define FILE_LOG_H

#ifndef RAYLIB_H
typedef enum {
    LOG_ALL = 0,        // Display all logs
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_NONE            // Disable logging
} log_level_e;
#endif

void trace_log(int log_level, const char* fmt, ...);

#endif // FILE_LOG_H
