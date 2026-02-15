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

// instanceMap (global)
static struct ViewMap **instanceMap = NULL;

// mutex for viewMap (global)
static Mutex instanceMapMutex;
static MutexOnce mutexOnce = MUTEX_ONCE_INIT;

static void initializeView(struct IView *self, struct ObserverMap **observerMap, struct MediatorMap **mediatorMap) {
    struct View *this = (struct View *) self;
    this->observerMap = observerMap;
    this->mediatorMap = mediatorMap;
}

// api change since the search needs to happen on the pre-allocated slots for an empty or a new entry and instantiate it
static bool registerObserver(struct IView *self, const char *notificationName, bool (*notify)(const void *context, const struct INotification *notification), void *context) {
    struct View *this = (struct View *) self;
    bool registered = false;

    mutex_lock(&this->observerMapMutex);

    if (this->observerMap == NULL) { // missing ObserverMap field
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] ERROR: Missing ObserverMap field in ViewMap; skipping registration.\033[0m\n");
        goto finally;
    }

    size_t i = 0;
    for (; this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find existing
        if (strcmp(this->observerMap[i]->key, notificationName) == 0) { // match
            struct IObserver **observers = this->observerMap[i]->observers;

            size_t j = 0; // find available observer slot
            for (; observers[j] != NULL && observers[j]->getContext != NULL; j++) {}

            if (observers[j] == NULL) { // overflow (Observer)
                fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] ERROR: Observer storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
                goto finally;
            }

            puremvc_observer_init(observers[j], notify, context); // init (existing key)
            registered = true;
            goto finally;
        }
    }

    if (this->observerMap[i] == NULL) { // overflow (ObserverMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: ObserverMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        goto finally;
    }

    if (this->observerMap[i]->observers == NULL || this->observerMap[i]->observers[0] == NULL) { // overflow (Observer)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: Observer storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        goto finally;
    }

    puremvc_observer_init(this->observerMap[i]->observers[0], notify, context);

    int len = snprintf(this->observerMap[i]->key, KEY_SIZE, "%s", notificationName); // init (new key)
    if (len < 0 || len >= KEY_SIZE) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: ObserverMap key truncated: '%s' (max %d chars).\033[0m\n", notificationName, KEY_SIZE);
        memset(this->observerMap[i]->key, 0, KEY_SIZE);
        goto finally;
    }

    registered = true;

finally:
    mutex_unlock(&this->observerMapMutex);
    return registered;
}

static void notifyObservers(const struct IView *self, const struct INotification *notification) {
    if (notification == NULL) return;
    struct View *this = (struct View *) self;

    mutex_lock_shared(&this->observerMapMutex);
    struct IObserver **observers = NULL;
    for (size_t i = 0; this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find observer
        if (strcmp(this->observerMap[i]->key, notification->getName(notification)) == 0) {
            observers = this->observerMap[i]->observers;
            break;
        }
    }
    mutex_unlock(&this->observerMapMutex);

    if (observers == NULL) return;
    for (size_t i = 0; observers[i] != NULL; i++) {
        const struct IObserver *observer = observers[i];
        observer->notifyObserver(observer, notification);
    }
}

bool removeObserver(struct IView *self, const char *notificationName, const void *notifyContext) {
    struct View *this = (struct View *) self;
    bool removed = false;

    mutex_lock(&this->observerMapMutex);

    // todo after sanitization remove extra NULL check
    for (size_t i = 0; this->observerMap != NULL && this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find observer
        if (strcmp(this->observerMap[i]->key, notificationName) == 0) { // match (observerMap key)

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

            if (index == 0) { // Since no entries were shifted left, the current observerMap is empty
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
    struct IMediator *mediator = NULL;
    bool registered = false;

    mutex_lock(&this->mediatorMapMutex);

    if (this->mediatorMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] ERROR: Missing MediatorMap field in ViewMap; skipping registration.\033[0m\n");
        goto finally;
    }

    size_t i = 0;
    for (; this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find existing
        if (strcmp(this->mediatorMap[i]->key, name) == 0) { // no override; return
#ifndef NDEBUG
            printf("\033[0;33m[PureMVC::View::registerMediator] Warning: Mediator '%s' exists; skipping registration\033[0m.\n", name);
#endif
            goto finally;
        }
    }

    if (this->mediatorMap[i] == NULL) { // overflow (MediatorMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: MediatorMap storage overflow for mediator '%s'; increase slots - skipping registration.\033[0m\n", name);
        goto finally;
    }

    mediator = factory(this->mediatorMap[i]->mediator, name, component); // init todo check if mediator exists (error if .mediator wasn't alloca)
    mediator->getNotifier(mediator)->initializeNotifier(mediator->getNotifier(mediator), this->multitonKey);
    mediator->onRegister(mediator);

    const char *const *interests = mediator->listNotificationInterests(mediator);
    for (const char *const *interest = interests; *interest; interest++) { // register observers (mutex guards context if mediator is removed)
        self->registerObserver(self, *interest, (bool (*)(const void *, const struct INotification *)) mediator->handleNotification, mediator);
    }

    const char *key = mediator->getName(mediator);
    int len = snprintf(this->mediatorMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len < 0 || len >= KEY_SIZE) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: MediatorMap key truncated: '%s' (max %zu chars).\033[0m\n", this->mediatorMap[i]->key, sizeof(key));
        memset(this->mediatorMap[i]->key, 0, KEY_SIZE); // clear
        goto finally;
    }

    registered = true;

finally:
    mutex_unlock(&this->mediatorMapMutex);
    return registered;
}

static struct IMediator *retrieveMediator(const struct IView *self, const char *mediatorName) {
    if (mediatorName == NULL) return NULL;
    struct View *this = (struct View *) self;
    struct IMediator *mediator = NULL;

    mutex_lock_shared(&this->mediatorMapMutex);
    for (size_t i = 0; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->mediatorMap[i]->key, mediatorName) == 0) {
            mediator = this->mediatorMap[i]->mediator;
            break;
        }
    }
    mutex_unlock(&this->mediatorMapMutex);

    return mediator;
}

static bool hasMediator(const struct IView *self, const char *mediatorName) {
    if (mediatorName == NULL) return false;
    struct View *this = (struct View *) self;
    bool exists = false;

    mutex_lock_shared(&this->mediatorMapMutex);
    for (size_t i = 0; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->mediatorMap[i]->key, mediatorName) == 0) {
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

    for (size_t i = 0, index = 0; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find mediator
        if (strcmp(this->mediatorMap[i]->key, mediatorName) == 0) { // match
            if (out != NULL) *out = this->mediatorMap[i]->mediator; // out param

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

                struct IMediator *mediator = this->mediatorMap[index]->mediator;
                const char *const *interests = mediator->listNotificationInterests(mediator);
                for (const char *const *cursor = interests; *cursor; cursor++) { // update observer context to relocated mediators
                    for (size_t j = 0; this->observerMap[j] != NULL && this->observerMap[j]->key[0] != '\0'; j++) {
                        if (strcmp(this->observerMap[j]->key, *cursor) == 0) {
                            struct IObserver **observers = this->observerMap[j]->observers;
                            for (size_t k = 0; observers[k] != NULL && observers[k]->getContext(observers[k]) != NULL; k++) {
                                if (observers[k]->getContext(observers[k]) == previous) {
                                    observers[k]->setContext(observers[k], mediator); // point context to mediator's new address
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
}

size_t puremvc_view_size() {
    return (sizeof(struct View) + (sizeof(void *) - 1u)) & ~(sizeof(void *) - 1u);
}

struct IView *puremvc_view_init(void *buffer, const char *key) {
    struct View *this = (struct View *) buffer;

    memset(this, 0, sizeof(struct View));

    this->super.initializeView = initializeView;
    this->super.registerObserver = registerObserver;
    this->super.notifyObservers = notifyObservers;
    this->super.removeObserver = removeObserver;
    this->super.registerMediator = registerMediator;
    this->super.retrieveMediator = retrieveMediator;
    this->super.hasMediator = hasMediator;
    this->super.removeMediator = removeMediator;

    int len = snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    if (len < 0 || len >= KEY_SIZE) {
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
     mutex_init(&instanceMapMutex);
}

struct IView *puremvc_view_getInstance(struct ViewMap **viewMap, const char *key) {
    if (viewMap == NULL || key == NULL) return NULL;
    struct IView *view = NULL;

    instanceMap = viewMap;

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t i = 0;
    for (; instanceMap != NULL && instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find view
        if (strcmp(instanceMap[i]->key, key) == 0) {
            view = instanceMap[i]->view;
            goto finally;
        }
    }

    if (instanceMap == NULL || instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] ERROR: ViewMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        goto finally;
    }

    if (instanceMap[i]->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] ERROR: Missing View storage; skipping registration.\033[0m\n");
        goto finally;
    }

    int len = snprintf(instanceMap[i]->key, KEY_SIZE, "%s", key); // init
    if (len < 0 || len >= KEY_SIZE) { // tod reset view?
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] Error: ViewMap key truncated: '%s' (max %zu chars).\033[0m\n", key, sizeof(key));
        memset(instanceMap[i]->key, 0, KEY_SIZE);
        goto finally;
    }

    view = puremvc_view_init(instanceMap[i]->view, key); // init

finally:
    mutex_unlock(&instanceMapMutex);
    return view;
}

bool puremvc_view_removeView(const char *key, struct IView **out) {
    if (key == NULL) return false;
    bool removed = false;

    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::removeView] ERROR: Missing ViewMap storage; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t index = 0;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find view
        if (strcmp(instanceMap[i]->key, key) == 0) {
            if (out != NULL) *out = instanceMap[i]->view;
            memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i) { // shift left (Gap-free array)
                *instanceMap[index] = *instanceMap[i]; // shift left first
                memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            }
            index++;
        }
    }

    if (index == 0) instanceMap = NULL; // avoid dangling global stack pointer after removal of last entry

    mutex_unlock(&instanceMapMutex);
    return removed;
}
