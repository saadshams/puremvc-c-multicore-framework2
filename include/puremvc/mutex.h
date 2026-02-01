/**
 * @file mutex.h
 * @brief Cross-platform mutex and thread abstraction.
 */
#pragma once

#ifdef _WIN32
#include <windows.h>

typedef struct { CRITICAL_SECTION cs; } Mutex;
typedef INIT_ONCE MutexOnce;

#define MUTEX_ONCE_INIT INIT_ONCE_STATIC_INIT

#else
#include <pthread.h>

typedef struct { pthread_rwlock_t rwlock; } Mutex;
typedef pthread_once_t MutexOnce;

#define MUTEX_ONCE_INIT PTHREAD_ONCE_INIT

#endif

void mutex_init(Mutex *m);
void mutex_lock(Mutex *m);
void mutex_lock_shared(Mutex *m);
void mutex_unlock(Mutex *m);
void mutex_destroy(Mutex *m);

void mutex_once(MutexOnce *once, void (*fn)(void));
