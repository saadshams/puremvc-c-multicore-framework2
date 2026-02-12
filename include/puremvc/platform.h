/**
 * @file platform.h
 * @brief Cross-platform mutex and thread abstraction.
 */
#pragma once

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>

#define alloca _alloca // MSVC sometimes prefers _alloca, this maps the standard name to it

typedef struct { CRITICAL_SECTION cs; } Mutex;
typedef INIT_ONCE MutexOnce;

#define MUTEX_ONCE_INIT INIT_ONCE_STATIC_INIT

#else
#include <pthread.h>
#include <alloca.h>

typedef struct { pthread_rwlock_t rwlock; } Mutex;
typedef pthread_once_t MutexOnce;

#define MUTEX_ONCE_INIT PTHREAD_ONCE_INIT

#endif

int mutex_init(Mutex *mutex);
int mutex_lock(Mutex *mutex);
int mutex_lock_shared(Mutex *mutex);
int mutex_unlock(Mutex *mutex);
int mutex_destroy(Mutex *mutex);

int mutex_once(MutexOnce *once, void (*callback)(void));
