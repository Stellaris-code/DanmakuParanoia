/*
 * <32021 by Stellaris. Copying Art is an act of love. Love is not subject to law.
 */
#ifndef STATE_H
#define STATE_H

// At most 4 states can be running on top of each others
#define MAX_ACTIVE_STATES 4

typedef struct state_t
{
    void (*init)(struct state_t*);
    void (*exit)(struct state_t*);
    void (*update)(struct state_t*, float dt);
    void (*render)(struct state_t*);
} state_t;

#endif // STATE_H
