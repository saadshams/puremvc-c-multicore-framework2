/**
 * @file platform.h
 * @brief Cross-platform mutex and thread abstraction.
 */
#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <malloc.h>
#define alloca _alloca // MSVC sometimes prefers _alloca, this maps the standard name to it

typedef struct { CRITICAL_SECTION cs; } Mutex;
typedef INIT_ONCE MutexOnce;
#define MUTEX_ONCE_INIT INIT_ONCE_STATIC_INIT

#else
#define _GNU_SOURCE
#include <pthread.h>
#include <alloca.h>

typedef struct { pthread_rwlock_t rwlock; } Mutex;
typedef pthread_once_t MutexOnce;
#define MUTEX_ONCE_INIT PTHREAD_ONCE_INIT

// Alignment Shim (The UBSan Fix)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #include <stdalign.h>
#else
/* Fallback for pre-C11 or specialized compilers */
    #ifndef alignas
        #if defined(_MSC_VER)
            #define alignas(x) __declspec(align(x))
        #elif defined(__GNUC__) || defined(__clang__)
            #define alignas(x) __attribute__((aligned(x)))
        #else
            #define alignas(x)
        #endif
    #endif
#endif

// Alignment to pointer size (8 bytes on 64-bit, 4 on 32-bit)
#define ALIGNMENT sizeof(void *)

#endif

// Mutex Function Prototypes
int mutex_init(Mutex *mutex);
int mutex_lock(Mutex *mutex);
int mutex_lock_shared(Mutex *mutex);
int mutex_unlock(Mutex *mutex);
int mutex_destroy(Mutex *mutex);

int mutex_once(MutexOnce *once, void (*callback)(void));
