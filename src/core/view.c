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
    size_t i = 0; // search map
    for (; i < OBSERVER_MAP_SIZE && self->observerMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->observerMap[i].key, notificationName) == 0) {
            size_t j = 0; // search array
            for (; j < OBSERVER_ARRAY_SIZE && self->observerMap[i].observers[j].context != NULL; j++) {}
            if (j >= OBSERVER_ARRAY_SIZE) return; // array is full
            self->observerMap[i].observers[j] = observer; // append
            return;
        }
    }

    if (i >= OBSERVER_MAP_SIZE) return; // observerMap is full

    snprintf(self->observerMap[i].key, NAME_SIZE, "%s", notificationName); // insert
    self->observerMap[i].observers[0] = observer;
    // mutex_unlock(&this->observerMapMutex);
}

static void notifyObservers(const struct View *self, struct Notification notification) {
    // mutex_lock_shared(&this->observerMapMutex);
    struct Observer observers[OBSERVER_ARRAY_SIZE];

    size_t i = 0, j = 0;
    for (; i < OBSERVER_MAP_SIZE && self->observerMap[i].key[0] != '\0'; i++) { // search map
        if (strcmp(self->observerMap[i].key, notification.name) == 0) {
            for (; j < OBSERVER_ARRAY_SIZE && self->observerMap[i].observers[j].context != NULL; j++) {}
            memcpy(observers, self->observerMap[i].observers, j * sizeof(struct Observer));
            break;
        }
    }
    // mutex_unlock(&this->observerMapMutex);

    for (i = 0; i < j; i++)
        observers[i].notifyObserver(&observers[i], notification);
}

void removeObserver(struct View *self, const char *notificationName, const void *notifyContext) {
    // mutex_lock(&this->observerMapMutex);
    for (size_t i = 0; i < OBSERVER_MAP_SIZE && self->observerMap[i].key[0] != '\0'; i++) { // search map
        if (strcmp(self->observerMap[i].key, notificationName) == 0) { // match

            size_t index = 0; // One-pass removal (Filter pattern)
            size_t j = 0;
            for (; j < OBSERVER_ARRAY_SIZE && self->observerMap[i].observers[j].context != NULL; j++) { // search array
                const struct Observer observer = self->observerMap[i].observers[j];
                if (observer.compareNotifyContext(&observer, notifyContext) == true) { // match, skip
                    memset(&self->observerMap[i].observers[j], 0, sizeof(struct Observer));
                } else {
                    if (index != j) { // shift left
                        memmove(&self->observerMap[i].observers[j], &self->observerMap[i].observers[j+1], sizeof(struct Observer));
                    }
                    index++;
                }
            }

            if (j == 0) { // empty observers
                for (size_t k = i; k < OBSERVER_MAP_SIZE; k++) { // shift left loop
                    memmove(&self->observerMap[k], &self->observerMap[k+1], sizeof(struct ObserverMap));
                }
            }
            // mutex_unlock(&this->observerMapMutex);
            return;
        }
    }
    // mutex_unlock(&this->observerMapMutex);
}

static void registerMediator(struct View *self, struct Mediator mediator) {
    // mutex_lock(&this->mediatorMapMutex);
    size_t i = 0;
    for (; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) { // search
        if (strcmp(self->mediatorMap[i].key, mediator.getName(&mediator)) == 0) return; // no upsert
    }

    if (i >= MEDIATOR_MAP_SIZE) return; // mediatorMap is full

    mediator.notifier.initializeNotifier(&mediator.notifier, self->multitonKey);

    snprintf(self->mediatorMap[i].key, NAME_SIZE, "%s", mediator.name); // insert
    self->mediatorMap[i].mediator = mediator;
    // mutex_unlock(&this->mediatorMapMutex);

    const char **interests = self->mediatorMap[i].mediator.listNotificationInterests(&self->mediatorMap[i].mediator);
    for (const char **interest = interests; *interest; interest++) {
        const struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) self->mediatorMap[i].mediator.handleNotification, &self->mediatorMap[i].mediator);
        self->registerObserver(self, *interest, observer);
    }
    self->mediatorMap[i].mediator.onRegister(&self->mediatorMap[i].mediator);
}

static struct Mediator *retrieveMediator(struct View *self, const char *mediatorName) {
    // mutex_lock_shared(&this->mediatorMapMutex);
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) {
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
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) {
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
    struct Mediator value = {0};

    size_t index = 0; // One-pass removal (Filter pattern)
    size_t i = 0;
    for (; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) { // search
        if (strcmp(self->mediatorMap[i].key, mediatorName) == 0) {
            mediatorMap = &self->mediatorMap[i]; // found
        } else {
            if (index != i) { // shift left
                memmove(&self->mediatorMap[index], &self->mediatorMap[i], sizeof(struct Mediator));
                memset(&self->mediatorMap[i], 0, sizeof(struct MediatorMap));
            }
            index++;
        }
    }
    // mutex_unlock(&this->mediatorMapMutex);

    if (mediatorMap != NULL) {
        const char **interests = mediatorMap->mediator.listNotificationInterests(&mediatorMap->mediator);
        for (const char **cursor = interests; *cursor; cursor++) {
            self->removeObserver(self, *cursor, &mediatorMap->mediator);
        }
        mediatorMap->mediator.onRemove(&mediatorMap->mediator);
        value = mediatorMap->mediator;

        memset(mediatorMap, 0, sizeof(struct MediatorMap));
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
