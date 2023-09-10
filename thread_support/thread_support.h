#ifndef THREAD_SUPPORT_H
#define THREAD_SUPPORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {

    thrd_success = 0,
    thrd_nomem = 1,
    thrd_timedout = 2,
    thrd_busy = 3,
    thrd_error = 4
};

enum {

    mtx_plain = 0,
    mtx_recursive = 1,
    mtx_timed = 2
};

typedef uint32_t thrd_t;
typedef uint32_t mtx_t;
typedef uint32_t cnd_t;
typedef uint32_t async_tok_t;

typedef int(*thrd_start_t)(void*);
typedef int(*thrd_start_2_t)(void*, void*);
int thrd_create( thrd_t *thr, thrd_start_t func, void *arg );
int thrd_create_2( thrd_t *thr, thrd_start_2_t func, void *arg1, void* arg2 );
int thrd_join( thrd_t thr, int *res );
int thrd_hardware_threads();

int mtx_init(mtx_t* mutex, int type);
int mtx_lock(mtx_t* mutex);
int mtx_trylock(mtx_t* mutex);
int mtx_unlock(mtx_t* mutex);
void mtx_destroy(mtx_t* mutex);

int cnd_init(cnd_t* cnd, int type);
int cnd_signal(cnd_t* cnd);
int cnd_broadcast(cnd_t* cnd);
int cnd_wait(cnd_t* cnd, mtx_t* mutex);
void cnd_destroy(cnd_t* cnd);

// TODO : redo the implementation of the async functions
async_tok_t async_run(thrd_start_t func, void* arg);
async_tok_t async_run_2(thrd_start_2_t func, void* arg1, void* arg2);
int async_wait(async_tok_t tok);

#ifdef __cplusplus
}
#endif


#endif // THREAD_SUPPORT_H
