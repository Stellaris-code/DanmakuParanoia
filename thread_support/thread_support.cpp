#include "thread_support.h"

#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <cassert>

#define MAX_THREADS 256
#define MAX_MUTEXES 256
#define MAX_CNDS 256
#define INVALID_ID ((thrd_t)-1)

typedef struct mtx_entry_t
{
    bool active;
    std::mutex mutex;
} mtx_entry_t;


typedef struct cnd_entry_t
{
    bool active;
    std::condition_variable cnd;
} cnd_entry_t;

static std::thread threads[MAX_THREADS];
static mtx_entry_t mutexes[MAX_MUTEXES];
static std::future<int> async_toks[MAX_THREADS];
static cnd_entry_t cnds[MAX_CNDS];

static thrd_t alloc_tid()
{
    for (unsigned i = 0; i < MAX_THREADS; ++i)
    {
        if (!threads[i].joinable())
            return i;
    }

    return INVALID_ID;
}

static async_tok_t alloc_async_tok()
{
    for (unsigned i = 0; i < MAX_THREADS; ++i)
    {
        if (!async_toks[i].valid())
            return i;
    }

    return INVALID_ID;
}

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
    thrd_t id = alloc_tid();
    if (id == INVALID_ID)
        return thrd_nomem;

    threads[id] = std::thread(func, arg);
    *thr = id;

    return thrd_success;
}

int thrd_create_2(thrd_t *thr, thrd_start_2_t func, void *arg1, void *arg2)
{
    thrd_t id = alloc_tid();
    if (id == INVALID_ID)
        return thrd_nomem;

    threads[id] = std::thread(func, arg1, arg2);
    *thr = id;

    return thrd_success;
}

int thrd_join(thrd_t thr, int *res)
{
    if (thr == INVALID_ID || thr >= MAX_THREADS)
        return thrd_error;
    if (!threads[thr].joinable())
        return thrd_error;

    threads[thr].join();
    if (res)
        *res = 0;

    return thrd_success;
}

int thrd_hardware_threads()
{
    return std::thread::hardware_concurrency();
}

int mtx_init(mtx_t *mutex, int type)
{
    // type is ignored
    (void)type;

    for (unsigned i = 0; i < MAX_MUTEXES; ++i)
    {
        if (!mutexes[i].active)
        {
            mutexes[i].active = true;
            *mutex = i;
            return thrd_success;
        }
    }
    return thrd_error;
}

int mtx_lock(mtx_t *mutex)
{
    assert(*mutex >= 0 && *mutex < MAX_MUTEXES);
    assert(mutexes[*mutex].active);
    mutexes[*mutex].mutex.lock();
    return thrd_success;
}

int mtx_trylock(mtx_t *mutex)
{
    assert(*mutex >= 0 && *mutex < MAX_MUTEXES);
    assert(mutexes[*mutex].active);
    return mutexes[*mutex].mutex.try_lock() ? thrd_success : thrd_busy;
}

int mtx_unlock(mtx_t *mutex)
{
    assert(*mutex >= 0 && *mutex < MAX_MUTEXES);
    assert(mutexes[*mutex].active);
    mutexes[*mutex].mutex.unlock();
    return thrd_success;
}

void mtx_destroy(mtx_t *mutex)
{
    assert(*mutex >= 0 && *mutex < MAX_MUTEXES);
    assert(mutexes[*mutex].active);
    mutexes[*mutex].active = false;
}

int cnd_init(cnd_t *cnd, int type)
{
    // type is ignored
    (void)type;

    for (unsigned i = 0; i < MAX_CNDS; ++i)
    {
        if (!cnds[i].active)
        {
            cnds[i].active = true;
            *cnd = i;
            return thrd_success;
        }
    }
    return thrd_error;
}

int cnd_signal(cnd_t *cnd)
{
    assert(*cnd >= 0 && *cnd < MAX_CNDS);
    assert(cnds[*cnd].active);
    cnds[*cnd].cnd.notify_one();
    return thrd_success;
}

int cnd_broadcast(cnd_t *cnd)
{
    assert(*cnd >= 0 && *cnd < MAX_CNDS);
    assert(cnds[*cnd].active);
    cnds[*cnd].cnd.notify_all();
    return thrd_success;
}

int cnd_wait(cnd_t* cnd, mtx_t* mutex)
{
    assert(*cnd >= 0 && *cnd < MAX_CNDS);
    assert(cnds[*cnd].active);
    assert(*mutex >= 0 && *mutex < MAX_MUTEXES);
    assert(mutexes[*mutex].active);
    std::unique_lock<std::mutex> mtx_lock(mutexes[*mutex].mutex);
    cnds[*cnd].cnd.wait(mtx_lock);
    return thrd_success;
}

void cnd_destroy(cnd_t *cnd)
{
    assert(*cnd >= 0 && *cnd < MAX_CNDS);
    assert(cnds[*cnd].active);
    cnds[*cnd].active = false;
}

async_tok_t async_run(thrd_start_t func, void *arg)
{
    async_tok_t tok = alloc_async_tok();
    async_toks[tok] = std::async(std::launch::async, func, arg);
    assert(tok != INVALID_ID);

    return tok;
}

async_tok_t async_run_2(thrd_start_2_t func, void *arg1, void *arg2)
{
    async_tok_t tok = alloc_async_tok();
    async_toks[tok] = std::async(std::launch::async, func, arg1, arg2);
    assert(tok != INVALID_ID);

    return tok;
}

int async_wait(async_tok_t tok)
{
    assert(tok >= 0 && tok < MAX_THREADS);
    assert(async_toks[tok].valid());

    return async_toks[tok].get();
}
