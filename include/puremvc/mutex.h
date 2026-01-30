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

typedef HANDLE pthread_t;

typedef struct {
    void* (*start_routine)(void*);
    void* arg;
    HANDLE signal; // Used to signal that data has been copied
} _pthread_args;

static DWORD WINAPI _pthread_win_start(LPVOID arg) {
    _pthread_args* args = (_pthread_args*)arg;
    void* (*start_routine)(void*) = args->start_routine;
    void* real_arg = args->arg;

    // Signal the parent thread that we have captured the arguments
    SetEvent(args->signal);

    start_routine(real_arg);
    return 0;
}

static inline int pthread_create(pthread_t* thread, const void* attr, void* (*start_routine)(void*), void* arg) {
    (void)attr;
    _pthread_args args;
    args.start_routine = start_routine;
    args.arg = arg;

    // Create an auto-reset event to synchronize startup
    args.signal = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!args.signal) return -1;

    *thread = CreateThread(NULL, 0, _pthread_win_start, &args, 0, NULL);

    if (!*thread) {
        CloseHandle(args.signal);
        return -1;
    }

    // Wait until the child thread has copied the stack-allocated 'args'
    WaitForSingleObject(args.signal, INFINITE);
    CloseHandle(args.signal);

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