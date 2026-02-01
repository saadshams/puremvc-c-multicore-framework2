#include <pthread.h>
#include <stdlib.h>

#include "puremvc/mutex.h"

void mutex_init(Mutex *mutex) {
    pthread_rwlock_init(&mutex->rwlock, NULL);
}

void mutex_lock(Mutex *mutex) {
    pthread_rwlock_wrlock(&mutex->rwlock);
}

void mutex_lock_shared(Mutex *m) {
    pthread_rwlock_rdlock(&m->rwlock);
}

void mutex_unlock(Mutex *m) {
    pthread_rwlock_unlock(&m->rwlock);
}

void mutex_destroy(Mutex *m) {
    pthread_rwlock_destroy(&m->rwlock);
}

void mutex_once(MutexOnce *once, void (*fn)(void)) {
    pthread_once(once, fn);
}
