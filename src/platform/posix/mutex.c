#include "puremvc/platform.h"

#include <pthread.h>
#include <stdlib.h>

int mutex_init(Mutex *mutex) {
    if (mutex == NULL) return -1;
    return pthread_rwlock_init(&mutex->rwlock, NULL);
}

int mutex_lock(Mutex *mutex) {
    if (mutex == NULL) return -1;
    return pthread_rwlock_wrlock(&mutex->rwlock);
}

int mutex_lock_shared(Mutex *mutex) {
    if (mutex == NULL) return -1;
    return pthread_rwlock_rdlock(&mutex->rwlock);
}

int mutex_unlock(Mutex *mutex) {
    if (mutex == NULL) return -1;
    return pthread_rwlock_unlock(&mutex->rwlock);
}

int mutex_destroy(Mutex *mutex) {
    if (mutex == NULL) return -1;
    return pthread_rwlock_destroy(&mutex->rwlock);
}

int mutex_once(MutexOnce *once, void (*fn)(void)) {
    if (once == NULL || fn == NULL) return -1;
    return pthread_once(once, fn);
}
