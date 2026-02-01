#include <pthread.h>
#include <stdlib.h>

#include "puremvc/mutex.h"

void mutex_init(Mutex *mutex) {
    pthread_rwlock_init(&mutex->rwlock, NULL);
}

void mutex_lock(Mutex *mutex) {
    pthread_rwlock_wrlock(&mutex->rwlock);
}

void mutex_lock_shared(Mutex *mutex) {
    pthread_rwlock_rdlock(&mutex->rwlock);
}

void mutex_unlock(Mutex *mutex) {
    pthread_rwlock_unlock(&mutex->rwlock);
}

void mutex_destroy(Mutex *mutex) {
    pthread_rwlock_destroy(&mutex->rwlock);
}

void mutex_once(MutexOnce *once, void (*fn)(void)) {
    pthread_once(once, fn);
}
