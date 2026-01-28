/**
 * @file mutex.h
 * @ingroup Collection
 * @brief Cross-platform mutex abstraction for thread synchronization.
 *
 * Provides a unified interface for mutex operations on Windows and POSIX systems.
 * On Windows, it uses CRITICAL_SECTION for exclusive locking.
 * On POSIX systems, it uses pthread read-write locks with support for shared (read) and exclusive (write) locking.
 *
 * Macros:
 * - mutex_init(m): Initialize the mutex.
 * - mutex_destroy(m): Destroy the mutex and release resources.
 * - mutex_lock(m): Acquire exclusive (write) lock.
 * - mutex_unlock(m): Release the lock.
 * - mutex_lock_shared(m): Acquire shared (read) lock (equivalent to exclusive lock on Windows).
 *
 * Usage:
 * ```c
 * Mutex m;
 * mutex_init(&m);
 * mutex_lock(&m);
 * // critical section
 * mutex_unlock(&m);
 * mutex_destroy(&m);
 * ```
 *
 * @author Saad Shams https://linkedin.com/in/muizz
 * @copyright BSD 3-Clause License
 */

#pragma once

#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>

typedef struct { CRITICAL_SECTION cs; } Mutex;
#define mutex_init(m) InitializeCriticalSection(&(m)->cs)
#define mutex_destroy(m) DeleteCriticalSection(&(m)->cs)
#define mutex_lock(m) EnterCriticalSection(&(m)->cs)
#define mutex_unlock(m) LeaveCriticalSection(&(m)->cs)
#define mutex_lock_shared(m) mutex_lock(m) // Windows doesn't support rwlock natively, so read/write distinction is the same

typedef INIT_ONCE MutexOnce;
#define MUTEX_ONCE_INIT INIT_ONCE_STATIC_INIT

/*
 * This helper bridges the Windows InitOnce callback to a standard void function.
 * The init function is executed exactly once, and INIT_ONCE guarantees
 * proper synchronization and memory visibility for all waiting threads.
 */
typedef struct {
    void (*init)(void);
} MutexOnceContext;

static BOOL CALLBACK _mutex_win_once_wrapper(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context) {
    (void)InitOnce;
    (void)Context;
    void (*init)(void) = (void (*)(void))Parameter;
    init();
    return TRUE;
}

#define mutex_once(once_ptr, fn) \
    InitOnceExecuteOnce((once_ptr), _mutex_win_once_wrapper, (PVOID)(fn), NULL)

/* ---------- pthread ---------- */

typedef HANDLE pthread_t;

static DWORD WINAPI _pthread_win_start(LPVOID arg) {
    void** args = (void**)arg;
    void* (*start_routine)(void*) = (void* (*)(void*))args[0];
    void* real_arg = args[1];
    free(arg);
    start_routine(real_arg);
    return 0;
}

static inline int pthread_create(pthread_t* thread, const void* attr, void* (*start_routine)(void*), void* arg) {
    (void)attr;
    void** args = (void**)malloc(2 * sizeof(void*));
    if (!args) return -1;
    args[0] = (void*)start_routine;
    args[1] = arg;
    *thread = CreateThread(NULL, 0, _pthread_win_start, args, 0, NULL);
    if (!*thread) {
        free(args);
        return -1;
    }
    return 0;
}

static inline int pthread_join(pthread_t thread, void** retval) {
    (void)retval; /* return value not supported on Windows */
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

#else

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
