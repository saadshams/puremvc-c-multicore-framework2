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
static MutexOnce token = MUTEX_ONCE_INIT;
static Mutex mutex;

static void initializeView(struct View *self) {
    (void)self;
}

static void registerObserver(struct View *self, const char *notificationName, const struct Observer observer) {
    mutex_lock(&self->observerMapMutex);
    size_t i = 0;
    for (; i < OBSERVER_MAP_SIZE && self->observerMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->observerMap[i].key, notificationName) == 0) {
            size_t count = 0;
            while (count < OBSERVER_ARRAY_SIZE && self->observerMap[i].observers[count].context != NULL) count++;
            if (count >= OBSERVER_ARRAY_SIZE) {
                fprintf(stderr, "[PureMVC::View::registerObserver] Warning: Observers are at capacity for notification '%s' (max %d observers); skipping registration.\n", notificationName, OBSERVER_ARRAY_SIZE);
                mutex_unlock(&self->observerMapMutex);
                return;
            }
            self->observerMap[i].observers[count] = observer;
            mutex_unlock(&self->observerMapMutex);
            return;
        }
    }

    if (i >= OBSERVER_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::View::registerObserver] Warning: ObserverMap is at capacity for notification '%s' (max %d observers); skipping registration.\n", notificationName, OBSERVER_MAP_SIZE);
        mutex_unlock(&self->observerMapMutex);
        return;
    }

    snprintf(self->observerMap[i].key, NAME_SIZE, "%s", notificationName);
    self->observerMap[i].observers[0] = observer;
    mutex_unlock(&self->observerMapMutex);
}

static void notifyObservers(struct View *self, const struct Notification notification) {
    mutex_lock_shared(&self->observerMapMutex);
    struct Observer observers[OBSERVER_ARRAY_SIZE] = {0};
    size_t count = 0;
    for (size_t i = 0; i < OBSERVER_MAP_SIZE && self->observerMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->observerMap[i].key, notification.name) == 0) {
            while (count < OBSERVER_ARRAY_SIZE && self->observerMap[i].observers[count].context != NULL) count++;
            memcpy(observers, self->observerMap[i].observers, count * sizeof(struct Observer));
            break;
        }
    }
    mutex_unlock(&self->observerMapMutex);

    for (size_t i = 0; i < count; i++)
        observers[i].notifyObserver(&observers[i], notification);
}

void removeObserver(struct View *self, const char *notificationName, const void *notifyContext) {
    mutex_lock(&self->observerMapMutex);

    for (size_t i = 0; i < OBSERVER_MAP_SIZE && self->observerMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->observerMap[i].key, notificationName) == 0) {
            size_t index = 0;
            for (size_t j = 0; j < OBSERVER_ARRAY_SIZE && self->observerMap[i].observers[j].context != NULL; j++) {
                const struct Observer observer = self->observerMap[i].observers[j];
                if (observer.compareNotifyContext(&observer, notifyContext) == true) {
                    memset(&self->observerMap[i].observers[j], 0, sizeof(struct Observer));
                } else {
                    if (index != j) { // shift left
                        memmove(&self->observerMap[i].observers[index], &self->observerMap[i].observers[j], sizeof(struct Observer));
                        memset(&self->observerMap[i].observers[j], 0, sizeof(struct Observer));
                    }
                    index++;
                }
            }

            if (index == 0) { // empty observers
                memset(&self->observerMap[i], 0, sizeof(struct ObserverMap));
                for (size_t k = i; k < OBSERVER_MAP_SIZE - 1 && self->observerMap[k + 1].key[0] != '\0'; k++) { // shift left
                    memmove(&self->observerMap[k], &self->observerMap[k + 1], sizeof(struct ObserverMap));
                    memset(&self->observerMap[k + 1], 0, sizeof(struct ObserverMap));
                }
            }
            mutex_unlock(&self->observerMapMutex);
            return;
        }
    }
    mutex_unlock(&self->observerMapMutex);
}

static void registerMediator(struct View *self, struct Mediator mediator) {
    mutex_lock(&self->mediatorMapMutex);
    size_t i = 0;
    for (; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->mediatorMap[i].key, mediator.getName(&mediator)) == 0) {
            fprintf(stderr, "[PureMVC::View::registerMediator] Warning: Mediator '%s' exists; skipping registration.\n", mediator.getName(&mediator));
            mutex_unlock(&self->mediatorMapMutex);
            return;
        }
    }

    if (i >= MEDIATOR_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::View::registerMediator] Warning: MediatorMap is at capacity for mediator '%s' (max %d mediators); skipping registration.\n", mediator.name, OBSERVER_MAP_SIZE);
        mutex_unlock(&self->mediatorMapMutex);
        return;
    }

    mediator.notifier.initializeNotifier(&mediator.notifier, self->multitonKey);

    snprintf(self->mediatorMap[i].key, NAME_SIZE, "%s", mediator.name);
    self->mediatorMap[i].mediator = mediator;
    mutex_unlock(&self->mediatorMapMutex);

    const char **interests = self->mediatorMap[i].mediator.listNotificationInterests(&self->mediatorMap[i].mediator);
    for (const char **interest = interests; *interest; interest++) {
        const struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) self->mediatorMap[i].mediator.handleNotification, &self->mediatorMap[i].mediator);
        self->registerObserver(self, *interest, observer);
    }
    self->mediatorMap[i].mediator.onRegister(&self->mediatorMap[i].mediator);
}

static struct Mediator *retrieveMediator(struct View *self, const char *mediatorName) {
    mutex_lock_shared(&self->mediatorMapMutex);
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->mediatorMap[i].key, mediatorName) == 0) {
            mutex_unlock(&self->mediatorMapMutex);
            return &self->mediatorMap[i].mediator;
        }
    }
    mutex_unlock(&self->mediatorMapMutex);
    return NULL;
}

static bool hasMediator(struct View *self, const char *mediatorName) {
    mutex_lock_shared(&self->mediatorMapMutex);
    bool exists = false;
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->mediatorMap[i].key, mediatorName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&self->mediatorMapMutex);
    return exists;
}

static struct Mediator removeMediator(struct View *self, const char *mediatorName) {
    mutex_lock(&self->mediatorMapMutex);
    struct Mediator mediator = {0};

    size_t index = 0;
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && self->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->mediatorMap[i].key, mediatorName) == 0) {
            mediator = self->mediatorMap[i].mediator;
            const char **interests = self->mediatorMap[i].mediator.listNotificationInterests(&self->mediatorMap[i].mediator);
            for (const char **cursor = interests; *cursor; cursor++) {
                self->removeObserver(self, *cursor, &self->mediatorMap[i].mediator);
            }
            self->mediatorMap[i].mediator.onRemove(&self->mediatorMap[i].mediator);
            memset(&self->mediatorMap[i], 0, sizeof(struct MediatorMap));
        } else {
            if (index != i) { // shift left
                const char **interests = self->mediatorMap[i].mediator.listNotificationInterests(&self->mediatorMap[i].mediator);
                for (const char **cursor = interests; *cursor; cursor++) { // Remove observers to fix context before shifting mediators
                    self->removeObserver(self, *cursor, &self->mediatorMap[i].mediator);
                }
                memmove(&self->mediatorMap[index], &self->mediatorMap[i], sizeof(struct MediatorMap)); // shift mediator

                for (const char **cursor = interests; *cursor; cursor++) { // Re-register observers to new mediator's address
                    const struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) self->mediatorMap[index].mediator.handleNotification, &self->mediatorMap[index].mediator);
                    self->registerObserver(self, *cursor, observer);
                }
                memset(&self->mediatorMap[i], 0, sizeof(struct MediatorMap));
            }
            index++;
        }
    }

    mutex_unlock(&self->mediatorMapMutex);
    return mediator;
}

struct View puremvc_view(const char *key) {
    struct View view = {
        .initializeView = initializeView,
        .registerObserver = registerObserver,
        .notifyObservers = notifyObservers,
        .removeObserver = removeObserver,
        .registerMediator = registerMediator,
        .retrieveMediator = retrieveMediator,
        .hasMediator = hasMediator,
        .removeMediator = removeMediator
    };

    snprintf(view.multitonKey, KEY_SIZE, "%s", key);
    mutex_init(&view.observerMapMutex);
    mutex_init(&view.mediatorMapMutex);
    return view;
}

static void dispatchOnce() {
     mutex_init(&mutex);
}

struct View *puremvc_view_getInstance(const char *key, struct View(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&mutex);

    size_t i = 0;
    for (; instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            mutex_unlock(&mutex);
            return &instanceMap[i];
        }
    }

    if (i >= INSTANCE_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::View::getInstance] Warning: InstanceMap is at capacity for key '%s' (max %d instances); skipping registration.\n", key, INSTANCE_MAP_SIZE);
        mutex_unlock(&mutex);
        return NULL;
    }

    instanceMap[i] = factory(key);

    mutex_unlock(&mutex);
    return &instanceMap[i];
}

void puremvc_view_removeView(const char *key) {
    if (key == NULL) return;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&mutex);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            memset(&instanceMap[i], 0, sizeof(struct View));
        } else {
            if (index != i) {
                memmove(&instanceMap[index], &instanceMap[i], sizeof(struct View));
                memset(&instanceMap[i], 0, sizeof(struct View));
            }
            index++;
        }
    }
    mutex_unlock(&mutex);
}
