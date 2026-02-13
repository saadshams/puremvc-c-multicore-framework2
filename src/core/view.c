/**
* @file view.c
* @internal
* @brief View Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "view.h"

#include "puremvc/i_mediator.h"
#include "puremvc/i_observer.h"

#include <stdio.h>
#include <string.h>

// viewMap
static struct ViewMap **view_instanceMap = NULL;

// mutex for viewMap
static Mutex viewMapMutex;
static MutexOnce viewMutexOnce = MUTEX_ONCE_INIT;

static void initializeView(struct IView *self, struct ObserverMap **observerMap, struct MediatorMap **mediatorMap) {
    struct View *this = (struct View *) self;
    this->observerMap = observerMap;
    this->mediatorMap = mediatorMap;
}

// api change since the search needs to happen on the pre-allocated slots for an empty or a new entry and instantiate it
static bool registerObserver(struct IView *self, const char *notificationName, bool (*notify)(const void *context, const struct INotification *notification), void *context) {
    struct View *this = (struct View *) self;

    if (this->observerMap == NULL) { // missing ObserverMap field
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] FATAL: Missing ObserverMap field in ViewMap; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock(&this->observerMapMutex);

    size_t i = 0;
    for (; this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find existing
        if (this->observerMap[i]->key == notificationName || strcmp(this->observerMap[i]->key, notificationName) == 0) { // match
            struct IObserver **observers = this->observerMap[i]->observers;

            size_t j = 0; // find available observer slot
            for (; observers[j] != NULL && observers[j]->getContext != NULL; j++) {}

            if (observers[j] == NULL) { // overflow (Observer)
                fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] ERROR1: Observer storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
                mutex_unlock(&this->observerMapMutex);
                return false;
            }

            puremvc_observer_init(observers[j], notify, context); // init/registration (existing key)
            mutex_unlock(&this->observerMapMutex);
            return true;
        }
    }

    if (this->observerMap[i] == NULL) { // overflow (ObserverMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: ObserverMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->observerMapMutex);
        return false;
    }

    if (this->observerMap[i]->observers == NULL || this->observerMap[i]->observers[0] == NULL) { // overflow (Observer)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error2: Observer storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->observerMapMutex);
        return false;
    }

    puremvc_observer_init(this->observerMap[i]->observers[0], notify, context);

    int len = snprintf(this->observerMap[i]->key, KEY_SIZE, "%s", notificationName); // registration (new key)
    if (len >= KEY_SIZE) { // todo reset proxy
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: ObserverMap key truncated: '%s' (max %d chars).\033[0m\n", notificationName, KEY_SIZE);
        memset(this->observerMap[i]->key, 0, KEY_SIZE);
        mutex_unlock(&this->observerMapMutex);
        return false;
    }

    mutex_unlock(&this->observerMapMutex);
    return true;
}

static void notifyObservers(const struct IView *self, const struct INotification *notification) {
    // todo check for notification NULL
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->observerMapMutex);

    for (size_t i = 0; this->observerMap != NULL && this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find observer
        if (this->observerMap[i]->key == notification->getName(notification) || strcmp(this->observerMap[i]->key, notification->getName(notification)) == 0) {
            struct IObserver **observers = this->observerMap[i]->observers;
            for (size_t j = 0; observers != NULL && observers[j] != NULL; j++) {
                const struct IObserver *observer = this->observerMap[i]->observers[j];
                observer->notifyObserver(observer, notification);
            }
            break;
        }
    }

    mutex_unlock(&this->observerMapMutex);
}

bool removeObserver(struct IView *self, const char *notificationName, const void *notifyContext) {
    struct View *this = (struct View *) self;
    bool removed = false;

    mutex_lock(&this->observerMapMutex);

    // todo after sanitization remove extra NULL check
    for (size_t i = 0; this->observerMap != NULL && this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find observer
        if (this->observerMap[i]->key == notificationName || strcmp(this->observerMap[i]->key, notificationName) == 0) { // match (observerMap key)

            size_t index = 0, j = 0; // find observer to remove
            struct IObserver **observers = this->observerMap[i]->observers;
            for (; observers != NULL && observers[j] != NULL && observers[j]->getContext(observers[j]) != NULL; j++) {
                const struct IObserver *observer = observers[j];
                if (observer->compareNotifyContext(observer, notifyContext) == true) { // match
                    removed = true;
                    observers[j]->setContext(observers[j], NULL);
                } else {
                    if (index != j) { // shift observers left
                        observers[index]->setContext(observers[index], observers[j]->getContext(observers[j]));
                    }
                    index++;
                }
            }

            // mark remaining slots as dead
            for (size_t k = index; observers != NULL && observers[k] != NULL && observers[k]->getContext(observers[k]) != NULL; k++) {
                observers[k]->setContext(observers[k], NULL);
            }

            // todo why swap
            if (index == 0) { // Since no entries were shifted left, the current observerMap is empty (key check in the for loop has an effect)
                memset(this->observerMap[i]->key, 0, KEY_SIZE); // remove
                for (j = i; this->observerMap[j + 1] != NULL && this->observerMap[j + 1]->key[0] != '\0'; j++) { // shift observerMap left
                    struct ObserverMap *temp = this->observerMap[j];
                    this->observerMap[j] = this->observerMap[j + 1];
                    this->observerMap[j + 1] = temp;
                }
            }

            break; // handled match
        }
    }

    mutex_unlock(&this->observerMapMutex);
    return removed;
}

bool registerMediator(struct IView *self, struct IMediator *(*factory)(void *buffer, const char *name, void *component), const char *name, void *component) {
    struct View *this = (struct View *) self;

    mutex_lock(&this->mediatorMapMutex);

    if (this->mediatorMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] FATAL: Missing MediatorMap field in ViewMap; skipping registration.\033[0m\n");
        return false;
    }

    size_t i = 0;
    for (; this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find existing
        if (this->mediatorMap[i]->key == name || strcmp(this->mediatorMap[i]->key, name) == 0) { // no override; return
#ifndef NDEBUG
            printf("\033[0;33m[PureMVC::View::registerMediator] Warning: Mediator '%s' exists; skipping registration\033[0m.\n", name);
#endif
            mutex_unlock(&this->mediatorMapMutex);
            return false;
        }
    }

    if (this->mediatorMap[i] == NULL) { // overflow (MediatorMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: MediatorMap storage overflow for mediator '%s'; increase slots - skipping registration.\033[0m\n", name);
        mutex_unlock(&this->mediatorMapMutex);
        return false;
    }

    // todo check if mediator exists (error if .mediator wasn't alloca)
    struct IMediator *mediator = factory(this->mediatorMap[i]->mediator, name, component); // init
    if (mediator == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Mediator::registerMediator] Error: Factory failed to initialize Mediator '%s' .\033[0m\n", name);
        return false;
    }

    struct INotifier *notifier = mediator->getNotifier(mediator);
    notifier->initializeNotifier(notifier, this->multitonKey);

    const char *key = mediator->getName(mediator);
    int len = snprintf(this->mediatorMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len >= KEY_SIZE) {
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ProxyMap key truncated: '%s' (max %zu chars).\033[0m\n", this->mediatorMap[i]->key, sizeof(key));
        memset(this->mediatorMap[i]->key, 0, KEY_SIZE); /// clear
        puremvc_mediator_init(this->mediatorMap[i]->mediator, NULL, NULL);
        mutex_unlock(&this->mediatorMapMutex);
    }

    const char *const *interests = mediator->listNotificationInterests(mediator);
    for (const char *const *interest = interests; *interest; interest++) { // register observers (mutex guards context if mediator is removed)
        self->registerObserver(self, *interest, (bool (*)(const void *, const struct INotification *)) mediator->handleNotification, mediator);
    }
    mediator->onRegister(mediator);

    mutex_unlock(&this->mediatorMapMutex);
    return true;
}

static struct IMediator *retrieveMediator(const struct IView *self, const char *mediatorName) {
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->mediatorMapMutex);
    for (size_t i = 0; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) {
        if (this->mediatorMap[i]->key == mediatorName || strcmp(this->mediatorMap[i]->key, mediatorName) == 0) {
            mutex_unlock(&this->mediatorMapMutex);
            return this->mediatorMap[i]->mediator;
        }
    }
    mutex_unlock(&this->mediatorMapMutex);
    return NULL;
}

static bool hasMediator(const struct IView *self, const char *mediatorName) {
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->mediatorMapMutex);
    bool exists = false;
    for (size_t i = 0; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) {
        if (this->mediatorMap[i]->key == mediatorName || strcmp(this->mediatorMap[i]->key, mediatorName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->mediatorMapMutex);
    return exists;
}

static bool removeMediator(struct IView *self, const char *mediatorName, struct IMediator **out) {
    struct View *this = (struct View *) self;
    bool removed = false;

    mutex_lock(&this->mediatorMapMutex);

    size_t index = 0, i = 0;
    for (; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find mediator
        if (this->mediatorMap[i]->key == mediatorName || strcmp(this->mediatorMap[i]->key, mediatorName) == 0) { // match
            if (out != NULL) // out param
                *out = this->mediatorMap[i]->mediator;

            struct IMediator *mediator = this->mediatorMap[i]->mediator;
            const char *const *interests = mediator->listNotificationInterests(mediator);
            for (const char *const *cursor = interests; *cursor != NULL; cursor++) { // remove notification observers
                self->removeObserver(self, *cursor, mediator);
            }
            mediator->onRemove(mediator);

            memset(&this->mediatorMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i) { // shift mediatorMap left
                const struct IMediator *previous = this->mediatorMap[i]->mediator;

                *this->mediatorMap[index] = *this->mediatorMap[i]; // shift left first
                memset(&this->mediatorMap[i]->key, 0, KEY_SIZE); // remove

                const char *const *interests = this->mediatorMap[index]->mediator->listNotificationInterests(this->mediatorMap[index]->mediator);
                for (const char *const *cursor = interests; *cursor; cursor++) { // update observer context to relocated mediators
                    for (size_t j = 0; this->observerMap[j] != NULL && this->observerMap[j]->key[0] != '\0'; j++) {
                        if (this->observerMap[j]->key == *cursor || strcmp(this->observerMap[j]->key, *cursor) == 0) {
                            struct IObserver **observers = this->observerMap[j]->observers;
                            for (size_t k = 0; observers[k] != NULL && observers[k]->getContext(observers[k]) != NULL; k++) {
                                if (observers[k]->getContext(observers[k]) == previous) {
                                    observers[k]->setContext(observers[k], this->mediatorMap[index]->mediator); // point context to mediator's new address
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
    return removed;

    // mark remaining slots as dead
    // for (size_t k = index; this->mediatorMap != NULL && this->mediatorMap[k] != NULL && this->mediatorMap[k]->mediator != NULL; k++) {
    //     this->mediatorMap[k]->mediator = NULL; // questionable code, why setting NULL
    // also do we need to run the loop as i am re-initializing as i shift left
    // }
}

size_t puremvc_view_size() {
    return (sizeof(struct View) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct IView *puremvc_view_init(void *buffer, const char *key) {
    struct View *this = (struct View *) buffer;

    memset(this, 0, sizeof(struct View));

    this->base.initializeView = initializeView;
    this->base.registerObserver = registerObserver;
    this->base.notifyObservers = notifyObservers;
    this->base.removeObserver = removeObserver;
    this->base.registerMediator = registerMediator;
    this->base.retrieveMediator = retrieveMediator;
    this->base.hasMediator = hasMediator;
    this->base.removeMediator = removeMediator;

    int len = snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE) {
        memset(this, 0, sizeof(struct View));
        fprintf(stderr, "\033[0;31m[PureMVC::View::init] Error: View multitonKey truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        return NULL;
    }

    if (mutex_init(&this->observerMapMutex) != 0) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::init] ERROR: ObserverMapMutex initialization failed for key '%s'.\033[0m\n", key);
        return NULL;
    }

    if (mutex_init(&this->mediatorMapMutex) != 0) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::init] ERROR: MediatorMapMutex initialization failed for key '%s'.\033[0m\n", key);
        return NULL;
    }

    return (struct IView *) this;
}

static void dispatchOnce(void) {
     mutex_init(&viewMapMutex);
}

struct IView *puremvc_view_getInstance(struct ViewMap **viewMap, const char *key) {
    if (viewMap == NULL && view_instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Missing ViewMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    view_instanceMap = viewMap;

    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t i = 0;
    for (; view_instanceMap != NULL && view_instanceMap[i] != NULL && view_instanceMap[i]->key[0] != '\0'; i++) { // find view
        if (view_instanceMap[i]->key == key || strcmp(view_instanceMap[i]->key, key) == 0) {
            mutex_unlock(&viewMapMutex);
            return view_instanceMap[i]->view;
        }
    }

    if (view_instanceMap == NULL || view_instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: ViewMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&viewMapMutex);
        return NULL;
    }

    if (view_instanceMap[i]->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Missing View storage; skipping registration.\033[0m\n");
        return NULL;
    }

    int len = snprintf(view_instanceMap[i]->key, KEY_SIZE, "%s", key); // init
    if (len >= KEY_SIZE) { // tod reset view?
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] Error: ViewMap key truncated: '%s' (max %zu chars).\033[0m\n", key, sizeof(key));
        memset(view_instanceMap[i]->key, 0, KEY_SIZE);
        mutex_unlock(&viewMapMutex);
        return false;
    }

    puremvc_view_init(view_instanceMap[i]->view, key); // init

    mutex_unlock(&viewMapMutex);
    return view_instanceMap[i]->view;
}

bool puremvc_view_removeView(const char *key, struct IView **out) {
    bool removed = false;

    if (view_instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::removeView] FATAL: Missing ViewMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::removeView] FATAL: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t index = 0;
    for (size_t i = 0; view_instanceMap[i] != NULL && view_instanceMap[i]->key[0] != '\0'; i++) { // find view
        if (view_instanceMap[i]->key == key || strcmp(view_instanceMap[i]->key, key) == 0) {
            memset(view_instanceMap[i]->key, 0, KEY_SIZE); // remove
            if (out != NULL)
                *out = view_instanceMap[i]->view;

            removed = true;
        } else {
            if (index != i) { // shift left (Gap-free array)
                *view_instanceMap[index] = *view_instanceMap[i]; // shift left first
                memset(view_instanceMap[i]->key, 0, KEY_SIZE); // remove
            }
            index++;
        }
    }

    mutex_unlock(&viewMapMutex);

    return removed;
}

void puremvc_view_reset() {
    mutex_lock(&viewMapMutex);
    view_instanceMap = NULL;
    mutex_unlock(&viewMapMutex);
}
