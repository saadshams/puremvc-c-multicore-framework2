#include "puremvc/platform.h"

#include <stdlib.h>

int mutex_init(Mutex *mutex) {
    if (mutex == NULL) return -1;
    InitializeCriticalSection(&mutex->cs);
    return 0;
}

int mutex_lock(Mutex *mutex) {
    if (mutex == NULL) return -1;
    EnterCriticalSection(&mutex->cs);
    return 0;
}

int mutex_lock_shared(Mutex *mutex) {
    if (mutex == NULL) return -1;
    EnterCriticalSection(&mutex->cs);
    return 0;
}

int mutex_unlock(Mutex *mutex) {
    if (mutex == NULL) return -1;
    LeaveCriticalSection(&mutex->cs);
    return 0;
}

int mutex_destroy(Mutex *mutex) {
    if (mutex == NULL) return -1;
    DeleteCriticalSection(&mutex->cs);
    return 0;
}

static BOOL CALLBACK _mutex_win_once_wrapper(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context) {
    (void)InitOnce; (void)Context;
    void (*init)(void) = (void(*)(void))Parameter;
    init();
    return TRUE;
}

int mutex_once(MutexOnce *once, void (*callback)(void)) {
    InitOnceExecuteOnce(once, _mutex_win_once_wrapper, (PVOID)callback, NULL);
    return 0;
}
