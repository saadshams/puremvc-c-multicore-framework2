/**
* @file view.c
* @internal
* @brief View Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>
#include <string.h>

#include "puremvc/view.h"

// The Multiton View instanceMap.
static struct View instanceMap[INSTANCE_MAP_SIZE];

// mutex for instanceMap
// static MutexOnce token = MUTEX_ONCE_INIT;
// static Mutex mutex;

static void initializeView(struct View *self) {

}

static void registerObserver(struct View *self, const char *notificationName, struct Observer observer) {
    // mutex_lock(&this->observerMapMutex);
    for (size_t i = 0; i < self->observersMapCount; i++) { // search existing key
        if (strcmp(self->observerMap[i].key, notificationName) == 0) {
            if (self->observerMap[i].observersCount < OBSERVERS_ARRAY_SIZE) { // insert
                self->observerMap[i].observers[self->observerMap[i].observersCount++] = observer;
            }
            return;
        }
    }

    if (self->observersMapCount >= OBSERVER_MAP_SIZE) return; // observerMap is full

    snprintf(self->observerMap[self->observersMapCount].key, NAME_SIZE, "%s", notificationName); // assigns a new key
    self->observerMap[self->observersMapCount].observers[self->observerMap[self->observersMapCount].observersCount++] = observer;
    self->observersMapCount++;
    // mutex_unlock(&this->observerMapMutex);
}

static void notifyObservers(const struct View *self, struct Notification notification) {
    // mutex_lock_shared(&this->observerMapMutex);
    struct Observer copy[OBSERVERS_ARRAY_SIZE];
    size_t copyCount = 0;

    for (size_t i = 0; i < self->observersMapCount; i++) { // search key
        if (strcmp(self->observerMap[i].key, notification.name) == 0) {
            memcpy(copy, self->observerMap[i].observers, self->observerMap[i].observersCount * sizeof(struct Observer));
            copyCount = self->observerMap[i].observersCount;
            break;
        }
    }
    // mutex_unlock(&this->observerMapMutex);

    for (size_t i = 0; i < copyCount; i++)
        copy[i].notifyObserver(&copy[i], notification);
}

void removeObserver(struct View *self, const char *notificationName, const void *notifyContext) {
    // mutex_lock(&this->observerMapMutex);
    for (size_t i = 0; i < self->observersMapCount; i++) { // iterate
        if (strcmp(self->observerMap[i].key, notificationName) == 0) { // key match

            for (size_t j = 0; j < self->observerMap[i].observersCount; j++) { // search
                const struct Observer observer = self->observerMap[i].observers[j];
                if (observer.compareNotifyContext(&observer, notifyContext) == true) { // check
                    for (size_t k = j; k < self->observerMap[i].observersCount; k++) { // shift left loop
                        memmove(&self->observerMap[i].observers[k], &self->observerMap[i].observers[k+1], sizeof(struct Observer)); // remove
                    }
                    self->observerMap[i].observersCount--;
                }
            }

            if (self->observerMap[i].observersCount == 0) { // empty observers
                for (size_t j = i; j < self->observersMapCount; j++) { // shift left loop
                    memmove(&self->observerMap[j], &self->observerMap[j+1], sizeof(struct ObserverMap)); // remove
                }
            }
            self->observersMapCount--;
            // mutex_unlock(&this->observerMapMutex);
            return;
        }
    }
    // mutex_unlock(&this->observerMapMutex);
}

static void registerMediator(struct View *self, struct Mediator mediator) {
    // mutex_lock(&this->mediatorMapMutex);
    for (size_t i = 0; i < self->mediatorsMapCount; i++) { // search existing
        if (strcmp(self->mediatorMap[i].key, mediator.getName(&mediator)) == 0) return;
    }

    struct MediatorMap *mediatorMap = &self->mediatorMap[self->mediatorsMapCount];
    mediator.notifier.initializeNotifier(&mediator.notifier, self->multitonKey);

    if (self->mediatorsMapCount < MEDIATORS_MAP_SIZE) { // insert
        snprintf(mediatorMap->key, NAME_SIZE, "%s", mediator.name);
        mediatorMap->mediator = mediator;
        self->mediatorsMapCount++;
    }
    // mutex_unlock(&this->mediatorMapMutex);

    const char **interests = mediatorMap->mediator.listNotificationInterests(&mediatorMap->mediator);
    for (const char **interest = interests; *interest; interest++) {
        struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) mediatorMap->mediator.handleNotification, &mediatorMap->mediator);
        self->registerObserver(self, *interest, observer);
    }
    mediatorMap->mediator.onRegister(&mediatorMap->mediator);
}

static struct Mediator *retrieveMediator(struct View *self, const char *mediatorName) {
    // mutex_lock_shared(&this->mediatorMapMutex);
    for (size_t i = 0; i < self->mediatorsMapCount; i++) {
        if (strcmp(self->mediatorMap[i].key, mediatorName) == 0) {
            return &self->mediatorMap[i].mediator;
        }
    }
    // mutex_unlock(&this->mediatorMapMutex);
    return NULL;
}

static bool hasMediator(const struct View *self, const char *mediatorName) {
    // mutex_lock_shared(&this->mediatorMapMutex);
    bool exists = false;
    for (size_t i = 0; i < self->mediatorsMapCount; i++) {
        if (strcmp(self->mediatorMap[i].key, mediatorName) == 0) {
            exists = true;
            break;
        }
    }
    // mutex_unlock(&this->mediatorMapMutex);
    return exists;
}

// suggestion is to return value but check implications with unit tests
// pointer is passing all the tests
static struct Mediator removeMediator(struct View *self, const char *mediatorName) {
    // mutex_lock(&this->mediatorMapMutex);
    struct MediatorMap *mediatorMap = NULL;
    struct Mediator *mediator = NULL;
    struct Mediator value = {0};

    size_t index = 0;
    for (size_t i = 0; i < self->mediatorsMapCount; i++) { // One-pass removal (Filter pattern)
        if (strcmp(self->mediatorMap[i].key, mediatorName) == 0) { // check
            mediatorMap = &self->mediatorMap[i];
            mediator = &self->mediatorMap[i].mediator;
            value = self->mediatorMap[i].mediator;
        } else {
            if (index != i) { // shift left
                memmove(&self->mediatorMap[index], &self->mediatorMap[i], sizeof(struct Mediator));
                memset(&self->mediatorMap[i], 0, sizeof(struct Mediator));
            }
            index++;
        }
    }
    // mutex_unlock(&this->mediatorMapMutex);

    if (mediator != NULL) {
        const char **interests = mediator->listNotificationInterests(mediator);
        for (const char **cursor = interests; *cursor; cursor++) {
            self->removeObserver(self, *cursor, mediator);
        }
        mediator->onRemove(mediator);
        value = *mediator;

        memset(mediatorMap, 0, sizeof(struct MediatorMap));
        self->mediatorsMapCount--;
    }

    return value;
}

struct View puremvc_view(const char *key) {
    struct View view = {0};

    snprintf(view.multitonKey, KEY_SIZE, "%s", key);

    view.initializeView = initializeView;
    view.registerObserver = registerObserver;
    view.notifyObservers = notifyObservers;
    view.removeObserver = removeObserver;
    view.registerMediator = registerMediator;
    view.retrieveMediator = retrieveMediator;
    view.hasMediator = hasMediator;
    view.removeMediator = removeMediator;

    return view;
}

// static void dispatchOnce() {
//     mutex_init(&mutex);
// }

struct View *puremvc_view_getInstance(const char *key, struct View(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    size_t i = 0;
    for (; instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            return &instanceMap[i];
        }
    }

    if (i >= INSTANCE_MAP_SIZE) return NULL;

    instanceMap[i] = factory(key);

    // mutex_unlock(&mutex);
    return &instanceMap[i];
}

void puremvc_view_removeView(const char *key) {
    if (key == NULL) return;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    for (size_t i = 0; i < INSTANCE_MAP_SIZE; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            memset(&instanceMap[i], 0, sizeof(struct View)); // remove

            for (size_t j = i + 1; j < INSTANCE_MAP_SIZE; j++) // shift left
                instanceMap[j-1] = instanceMap[j];
            // memmove(&instanceMap[i], &instanceMap[i + 1], sizeof(instanceMap[0]) * (INSTANCE_MAP_SIZE - i));
            break;
        }
    }
    // mutex_unlock(&mutex);
}
