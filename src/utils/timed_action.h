#ifndef TIMED_ACTION_H
#define TIMED_ACTION_H

#define MAX_TIMED_ACTIONS 256

void init_timed_actions();
void timed_actions_update(float dt);
void schedule_action(float time, void(*callback)(void*), void* arg);

#endif // TIMED_ACTION_H
