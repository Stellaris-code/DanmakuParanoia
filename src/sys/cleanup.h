#ifndef CLEANUP_H
#define CLEANUP_H

typedef void(*cleanup_callback)(void);

typedef enum cleanup_event
{
    FrameEnd = 0,
    GamestateEnd = 1,
    AppEnd = 2
} cleanup_event;

void register_cleanup(cleanup_callback callback, cleanup_event event);

void do_cleanup(cleanup_event event);

#endif // CLEANUP_H
