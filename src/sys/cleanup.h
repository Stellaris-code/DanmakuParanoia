#ifndef CLEANUP_H
#define CLEANUP_H

typedef void(*cleanup_callback)(void);

void register_cleanup(cleanup_callback callback);

void do_cleanup();

#endif // CLEANUP_H
