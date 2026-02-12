#include "puremvc/platform.h"

#include <stdlib.h>

void mutex_init(Mutex *mutex) {
    InitializeCriticalSection(&mutex->cs);
}

void mutex_lock(Mutex *mutex) {
    EnterCriticalSection(&mutex->cs);
}

void mutex_lock_shared(Mutex *mutex) {
    EnterCriticalSection(&mutex->cs);
}

void mutex_unlock(Mutex *mutex) {
    LeaveCriticalSection(&mutex->cs);
}

void mutex_destroy(Mutex *mutex) {
    DeleteCriticalSection(&mutex->cs);
}

static BOOL CALLBACK _mutex_win_once_wrapper(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context) {
    (void)InitOnce; (void)Context;
    void (*init)(void) = (void(*)(void))Parameter;
    init();
    return TRUE;
}

void mutex_once(MutexOnce *once, void (*callback)(void)) {
    InitOnceExecuteOnce(once, _mutex_win_once_wrapper, (PVOID)callback, NULL);
}
