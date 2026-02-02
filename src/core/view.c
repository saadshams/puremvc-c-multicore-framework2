/**
* @file view.c
* @internal
* @brief View Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/view.h"

#include <stdio.h>
#include <string.h>

// The Multiton viewMap.
static struct ViewMap {
    char key[KEY_SIZE];
    struct View view;
} viewMap[INSTANCE_MAP_SIZE];

// mutex for viewMap
static Mutex viewMapMutex;
static MutexOnce viewMutexOnce = MUTEX_ONCE_INIT;

static void initializeView(struct IView *self) {
    (void)self;
}

static void registerObserver(const struct IView *self, const char *notificationName, struct Observer observer) {
    struct View *this = (struct View *) self;
    mutex_lock(&this->observerMapMutex);
    size_t i = 0;
    for (; i < OBSERVER_MAP_SIZE && this->observerMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->observerMap[i].key, notificationName) == 0) {
            size_t count = 0;
            while (count < OBSERVER_ARRAY_SIZE && this->observerMap[i].observers[count].context != NULL) count++;
            if (count >= OBSERVER_ARRAY_SIZE) {
                fprintf(stderr, "[PureMVC::View::registerObserver] Warning: Observers are at capacity for notification '%s' (max %d observers); skipping registration.\n", notificationName, OBSERVER_ARRAY_SIZE);
                mutex_unlock(&this->observerMapMutex);
                return;
            }
            this->observerMap[i].observers[count] = observer;
            mutex_unlock(&this->observerMapMutex);
            return;
        }
    }

    if (i >= OBSERVER_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::View::registerObserver] Warning: ObserverMap is at capacity for notification '%s' (max %d observers); skipping registration.\n", notificationName, OBSERVER_MAP_SIZE);
        mutex_unlock(&this->observerMapMutex);
        return;
    }

    int len = snprintf(this->observerMap[i].key, KEY_SIZE, "%s", notificationName);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::View::registerObserver] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", notificationName, len, KEY_SIZE);

    this->observerMap[i].observers[0] = observer;
    mutex_unlock(&this->observerMapMutex);
}

static void notifyObservers(const struct IView *self, struct INotification *notification) {
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->observerMapMutex);
    struct Observer observers[OBSERVER_ARRAY_SIZE] = {0};
    size_t count = 0;
    for (size_t i = 0; i < OBSERVER_MAP_SIZE && this->observerMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->observerMap[i].key, notification->getName(notification)) == 0) {
            while (count < OBSERVER_ARRAY_SIZE && this->observerMap[i].observers[count].context != NULL) count++;
            memcpy(observers, this->observerMap[i].observers, count * sizeof(struct Observer));
            break;
        }
    }
    mutex_unlock(&this->observerMapMutex);

    for (size_t i = 0; i < count; i++)
        observers[i].base.notifyObserver(&observers[i].base, notification);
}

void removeObserver(const struct IView *self, const char *notificationName, const void *notifyContext) {
    struct View *this = (struct View *) self;
    mutex_lock(&this->observerMapMutex);

    for (size_t i = 0; i < OBSERVER_MAP_SIZE && this->observerMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->observerMap[i].key, notificationName) == 0) {
            size_t index = 0;
            for (size_t j = 0; j < OBSERVER_ARRAY_SIZE && this->observerMap[i].observers[j].context != NULL; j++) {
                const struct Observer observer = this->observerMap[i].observers[j];
                if (observer.base.compareNotifyContext(&observer.base, notifyContext) == true) {
                    memset(&this->observerMap[i].observers[j], 0, sizeof(struct Observer));
                } else {
                    if (index != j) { // shift left
                        memmove(&this->observerMap[i].observers[index], &this->observerMap[i].observers[j], sizeof(struct Observer));
                        memset(&this->observerMap[i].observers[j], 0, sizeof(struct Observer));
                    }
                    index++;
                }
            }

            if (index == 0) { // empty observers
                memset(&this->observerMap[i], 0, sizeof(struct ObserverMap));
                for (size_t k = i; k < OBSERVER_MAP_SIZE - 1 && this->observerMap[k + 1].key[0] != '\0'; k++) { // shift left
                    memmove(&this->observerMap[k], &this->observerMap[k + 1], sizeof(struct ObserverMap));
                    memset(&this->observerMap[k + 1], 0, sizeof(struct ObserverMap));
                }
            }
            mutex_unlock(&this->observerMapMutex);
            return;
        }
    }
    mutex_unlock(&this->observerMapMutex);
}

static void registerMediator(const struct IView *self, struct Mediator mediator) {
    struct View *this = (struct View *) self;
    mutex_lock(&this->mediatorMapMutex);
    size_t i = 0;
    for (; i < MEDIATOR_MAP_SIZE && this->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->mediatorMap[i].key, mediator.base.getName(&mediator.base)) == 0) {
            fprintf(stderr, "[PureMVC::View::registerMediator] Warning: Mediator '%s' exists; skipping registration.\n", mediator.base.getName(&mediator.base));
            mutex_unlock(&this->mediatorMapMutex);
            return;
        }
    }

    if (i >= MEDIATOR_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::View::registerMediator] Warning: MediatorMap is at capacity for mediator '%s' (max %d mediators); skipping registration.\n", mediator.name, OBSERVER_MAP_SIZE);
        mutex_unlock(&this->mediatorMapMutex);
        return;
    }

    // mediator.notifier.initializeNotifier(&mediator.notifier, this->multitonKey); // todo

    int len = snprintf(this->mediatorMap[i].key, KEY_SIZE, "%s", mediator.name);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::View::registerMediator] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", mediator.name, len, KEY_SIZE);

    this->mediatorMap[i].mediator = mediator;
    mutex_unlock(&this->mediatorMapMutex);

    const char **interests = this->mediatorMap[i].mediator.base.listNotificationInterests(&this->mediatorMap[i].mediator.base);
    for (const char **interest = interests; *interest; interest++) {
        const struct Observer observer = puremvc_observer((void (*)(const void *, struct INotification *)) this->mediatorMap[i].mediator.base.handleNotification, &this->mediatorMap[i].mediator);
        self->registerObserver(self, *interest, observer);
    }
    this->mediatorMap[i].mediator.base.onRegister(&this->mediatorMap[i].mediator.base);
}

static struct IMediator *retrieveMediator(const struct IView *self, const char *mediatorName) {
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->mediatorMapMutex);
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && this->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->mediatorMap[i].key, mediatorName) == 0) {
            mutex_unlock(&this->mediatorMapMutex);
            return &this->mediatorMap[i].mediator.base;
        }
    }
    mutex_unlock(&this->mediatorMapMutex);
    return NULL;
}

static bool hasMediator(const struct IView *self, const char *mediatorName) {
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->mediatorMapMutex);
    bool exists = false;
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && this->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->mediatorMap[i].key, mediatorName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->mediatorMapMutex);
    return exists;
}

static struct Mediator removeMediator(const struct IView *self, const char *mediatorName) {
    struct View *this = (struct View *) self;
    mutex_lock(&this->mediatorMapMutex);
    struct Mediator mediator = {0};

    size_t index = 0;
    for (size_t i = 0; i < MEDIATOR_MAP_SIZE && this->mediatorMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->mediatorMap[i].key, mediatorName) == 0) {
            mediator = this->mediatorMap[i].mediator;
            const char **interests = this->mediatorMap[i].mediator.base.listNotificationInterests(&this->mediatorMap[i].mediator.base);
            for (const char **cursor = interests; *cursor; cursor++) {
                this->base.removeObserver(self, *cursor, &this->mediatorMap[i].mediator);
            }
            this->mediatorMap[i].mediator.base.onRemove(&this->mediatorMap[i].mediator.base);
            memset(&this->mediatorMap[i], 0, sizeof(struct MediatorMap));
        } else {
            if (index != i) { // shift left
                const struct Mediator *previous = &this->mediatorMap[i].mediator;

                memmove(&this->mediatorMap[index], &this->mediatorMap[i], sizeof(struct MediatorMap)); // move to new position
                memset(&this->mediatorMap[i], 0, sizeof(struct MediatorMap)); // clear the now-vacant slot

                const char **interests = this->mediatorMap[index].mediator.base.listNotificationInterests(&this->mediatorMap[index].mediator.base);
                for (const char **cursor = interests; *cursor; cursor++) { // update observer context
                    for (size_t j = 0; j < OBSERVER_MAP_SIZE && this->observerMap[j].key[0] != '\0'; j++) {
                        if (strcmp(this->observerMap[j].key, *cursor) == 0) {
                            for (size_t k = 0; k < OBSERVER_ARRAY_SIZE && this->observerMap[j].observers[k].context != NULL; k++) {
                                if (this->observerMap[j].observers[k].context == previous) {
                                    this->observerMap[j].observers[k].context = &this->mediatorMap[index].mediator; // point context to mediator's new address
                                }
                            }
                            break;
                        }
                    }
                }
            }
            index++;
        }
    }

    mutex_unlock(&this->mediatorMapMutex);
    return mediator;
}

struct View puremvc_view(const char *key) {
    struct View view = {
        .base = (struct IView) {
            .initializeView = initializeView,
            .registerObserver = registerObserver,
            .notifyObservers = notifyObservers,
            .removeObserver = removeObserver,
            .registerMediator = registerMediator,
            .retrieveMediator = retrieveMediator,
            .hasMediator = hasMediator,
            .removeMediator = removeMediator
        },
    };

    int len = snprintf(view.multitonKey, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::View] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    return view;
}

static void dispatchOnce(void) {
     mutex_init(&viewMapMutex);
}

struct IView *puremvc_view_getInstance(const char *key, struct View(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t i = 0;
    for (; i < INSTANCE_MAP_SIZE && viewMap[i].key[0] != '\0'; i++) {
        if (strncmp(viewMap[i].key, key, KEY_SIZE) == 0) {
            mutex_unlock(&viewMapMutex);
            return &viewMap[i].view.base;
        }
    }

    if (i >= INSTANCE_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::View::getInstance] Warning: InstanceMap is at capacity for key '%s' (max %d instances); skipping registration.\n", key, INSTANCE_MAP_SIZE);
        mutex_unlock(&viewMapMutex);
        return NULL;
    }

    int len = snprintf(viewMap[i].key, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::View::getInstance] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    viewMap[i].view = factory(key);
    mutex_init(&viewMap[i].view.observerMapMutex);
    mutex_init(&viewMap[i].view.mediatorMapMutex);

    viewMap[i].view.base.initializeView(&viewMap[i].view.base);

    mutex_unlock(&viewMapMutex);
    return &viewMap[i].view.base;
}

void puremvc_view_removeView(const char *key) {
    if (key == NULL) return;
    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && viewMap[i].key[0] != '\0'; i++) {
        if (strcmp(viewMap[i].key, key) == 0) {
            memset(&viewMap[i], 0, sizeof(struct ViewMap));
        } else {
            if (index != i) {
                memmove(&viewMap[index], &viewMap[i], sizeof(struct ViewMap));
                memset(&viewMap[i], 0, sizeof(struct ViewMap));
            }
            index++;
        }
    }
    mutex_unlock(&viewMapMutex);
}
