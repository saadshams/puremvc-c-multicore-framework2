/**
 * @file mutex.h
 * @ingroup Collection
 * @brief Cross-platform mutex and thread abstraction.
 * ...
 */

#pragma once

#ifdef _WIN32
#include <windows.h>

/* ---------- Mutex ---------- */
typedef struct { CRITICAL_SECTION cs; } Mutex;
#define mutex_init(m) InitializeCriticalSection(&(m)->cs)
#define mutex_destroy(m) DeleteCriticalSection(&(m)->cs)
#define mutex_lock(m) EnterCriticalSection(&(m)->cs)
#define mutex_unlock(m) LeaveCriticalSection(&(m)->cs)
#define mutex_lock_shared(m) mutex_lock(m)

/* ---------- Once ---------- */
typedef INIT_ONCE MutexOnce;
#define MUTEX_ONCE_INIT INIT_ONCE_STATIC_INIT

static BOOL CALLBACK _mutex_win_once_wrapper(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context) {
    (void)InitOnce; (void)Context;
    void (*init)(void) = (void (*)(void))Parameter;
    init();
    return TRUE;
}

#define mutex_once(once_ptr, fn) \
    InitOnceExecuteOnce((once_ptr), _mutex_win_once_wrapper, (PVOID)(fn), NULL)

/* ---------- pthread Shim (Stack-based with Sync) ---------- */

/* ---------- Fixed pthread Shim ---------- */

typedef HANDLE pthread_t;

typedef struct {
    void* (*start_routine)(void*);
    void* arg;
} _pthread_args;

static DWORD WINAPI _pthread_win_start(LPVOID arg) {
    _pthread_args* args = (_pthread_args*)arg;

    // Copy data to local variables so we can free the heap memory immediately
    void* (*start_routine)(void*) = args->start_routine;
    void* real_arg = args->arg;

    free(args); // Clean up the heap allocation

    start_routine(real_arg);
    return 0;
}

static inline int pthread_create(pthread_t* thread, const void* attr, void* (*start_routine)(void*), void* arg) {
    (void)attr;

    // Use the heap instead of the stack to ensure the data lives long enough
    _pthread_args* args = (_pthread_args*)malloc(sizeof(_pthread_args));
    if (!args) return -1;

    args->start_routine = start_routine;
    args->arg = arg;

    *thread = CreateThread(NULL, 0, _pthread_win_start, args, 0, NULL);

    if (!*thread) {
        free(args);
        return -1;
    }

    return 0;
}

static inline int pthread_join(pthread_t thread, void** retval) {
    (void)retval;
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

#else
/* ---------- POSIX Implementation ---------- */
#include <pthread.h>

typedef pthread_rwlock_t Mutex;
#define mutex_init(m) pthread_rwlock_init(m, NULL)
#define mutex_destroy(m) pthread_rwlock_destroy(m)
#define mutex_lock(m) pthread_rwlock_wrlock(m)
#define mutex_unlock(m) pthread_rwlock_unlock(m)
#define mutex_lock_shared(m) pthread_rwlock_rdlock(m)

typedef pthread_once_t MutexOnce;
#define MUTEX_ONCE_INIT PTHREAD_ONCE_INIT
#define mutex_once(m, fn) pthread_once(m, fn)

#endif