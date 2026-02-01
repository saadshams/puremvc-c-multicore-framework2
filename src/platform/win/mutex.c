#include <windows.h>
#include <stdlib.h>

#include "puremvc/mutex.h"

void mutex_init(Mutex *m) {
    InitializeCriticalSection(&m->cs);
}

void mutex_lock(Mutex *m) {
    EnterCriticalSection(&m->cs);
}

void mutex_lock_shared(Mutex *m) {
    EnterCriticalSection(&m->cs);
}

void mutex_unlock(Mutex *m) {
    LeaveCriticalSection(&m->cs);
}

void mutex_destroy(Mutex *m) {
    DeleteCriticalSection(&m->cs);
}

static BOOL CALLBACK _mutex_win_once_wrapper(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context) {
    (void)InitOnce; (void)Context;
    void (*init)(void) = (void(*)(void))Parameter;
    init();
    return TRUE;
}

void mutex_once(MutexOnce *once, void (*fn)(void)) {
    InitOnceExecuteOnce(once, _mutex_win_once_wrapper, (PVOID)fn, NULL);
}
