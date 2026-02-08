/**
* @file view.c
* @internal
* @brief View Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/view.h"
#include "puremvc/mediator.h"
#include "puremvc/observer.h"

#include <stdio.h>
#include <string.h>

// viewMap
static struct ViewMap **s_viewMap = NULL;

// mutex for viewMap
static Mutex viewMapMutex;
static MutexOnce viewMutexOnce = MUTEX_ONCE_INIT;

static void initializeView(struct IView *self) {
    (void)self;
}

// api change since the search needs to happen on the user slots and instantiate it
static bool registerObserver(struct IView *self, const char *notificationName, void (*notify)(const void *context, const struct INotification *notification), void *context) {
    struct View *this = (struct View *) self;

    if (strlen(notificationName) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Observer::registerObserver] Error: notification name '%s' too long (max %d) — skipping registration.\n", notificationName, KEY_SIZE);
        return false;
    }

    if (this->observerMap == NULL) { // missing ObserverMap field
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] FATAL: Missing ObserverMap field in ViewMap; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock(&this->observerMapMutex);

    size_t i = 0;
    for (; this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find existing
        if (strcmp(this->observerMap[i]->key, notificationName) != 0) // mismatch // todo nest
            continue;

        struct IObserver **observers = this->observerMap[i]->observers;

        size_t j = 0; // find available observer slot
        for (; observers[j] != NULL && observers[j]->getContext != NULL && observers[j]->getContext(observers[j]) != NULL; j++) {}

        if (observers[j] == NULL) { // overflow (Observer)
            fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] ERROR: Observer storage overflow for notification '%s' at ObserverMap index %zu; increase slots - skipping registration.\033[0m\n", notificationName, i);
            mutex_unlock(&this->observerMapMutex);
            return false;
        }

        puremvc_observer_init(observers[j], notify, context); // registration (existing key)
        mutex_unlock(&this->observerMapMutex);
        return false;
    }

    if (this->observerMap[i] == NULL) { // overflow (ObserverMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: ObserverMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->observerMapMutex);
        return false;
    }

    if (this->observerMap[i]->observers == NULL || this->observerMap[i]->observers[0] == NULL) { // overflow (Observer)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: Observer storage overflow for notification '%s' at ObserverMap index %zu; increase slots - skipping registration.\033[0m\n", notificationName, i);
        mutex_unlock(&this->observerMapMutex);
        return false;
    }

    snprintf(this->observerMap[i]->key, KEY_SIZE, "%s", notificationName); // registration (new key)
    puremvc_observer_init(this->observerMap[i]->observers[0], notify, context);
    mutex_unlock(&this->observerMapMutex);
    return true;
}

static void notifyObservers(const struct IView *self, const struct INotification *notification) {
    // todo check for notification NULL
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->observerMapMutex);

    for (size_t i = 0; this->observerMap != NULL && this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find observer
        if (strcmp(this->observerMap[i]->key, notification->getName(notification)) == 0) {
            struct IObserver **observers = this->observerMap[i]->observers;
            for (size_t j = 0; observers != NULL && observers[j] != NULL && observers[j]->getContext(observers[j]) != NULL; j++) {
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

    // todo check if i need '\0' check
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

            // todo why swap
            if (index == 0) { // Since no entries were shifted left, the current observerMap is empty (key check in the for loop has an effect)
                memset(&this->observerMap[i]->key, 0, KEY_SIZE); // remove the key
                for (size_t j = i; this->observerMap[j + 1] != NULL && this->observerMap[j + 1]->key[0] != '\0'; j++) { // shift observerMap left
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

bool registerMediator(struct IView *self, struct IMediator *(*factory)(struct IMediator *mediator, const char *name, void *component), const char *name, void *component) {
    struct View *this = (struct View *) self;

    if (strlen(name) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: Key '%s' too long (max %d) — skipping registration.\033[0m\n", name, KEY_SIZE);
        return false;
    }

    if (this->mediatorMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] FATAL: Missing MediatorMap field in ViewMap; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock(&this->mediatorMapMutex);

    size_t i = 0;
    for (; this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find existing
        if (strcmp(this->mediatorMap[i]->key, name) == 0) { // match (no override; return;)
            fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: Mediator '%s' exists; skipping registration\033[0m.\n", name);
            mutex_unlock(&this->mediatorMapMutex);
            return false;
        }
    }

    if (this->mediatorMap[i] == NULL) { // overflow (MediatorMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: MediatorMap storage overflow for mediator '%s'; increase slots - skipping registration.\033[0m\n", name);
        mutex_unlock(&this->mediatorMapMutex);
        return false;
    }

    // todo check if mediator exists
    struct IMediator *mediator = factory(this->mediatorMap[i]->mediator, name, component); // registration
    snprintf(this->mediatorMap[i]->key, KEY_SIZE, "%s", mediator->getName(mediator));

    mediator->getNotifier(mediator)->initializeNotifier(mediator->getNotifier(mediator), this->multitonKey);

    const char **interests = mediator->listNotificationInterests(mediator);
    for (const char **interest = interests; *interest; interest++) { // register observers (mutex guards context if mediator is removed)
        // void (*notify)(const void *, const struct INotification *) = (void (*)(const void *, const struct INotification *)) this->mediatorMap[i]->mediator->handleNotification;
        // void *context = this->mediatorMap[i]->mediator;
        self->registerObserver(self, *interest, (void (*)(const void *, const struct INotification *)) mediator->handleNotification, mediator);
    }

    mutex_unlock(&this->mediatorMapMutex);

    mediator->onRegister(mediator);
    return true;
}

static struct IMediator *retrieveMediator(const struct IView *self, const char *mediatorName) {
    struct View *this = (struct View *) self;
    mutex_lock_shared(&this->mediatorMapMutex);
    for (size_t i = 0; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->mediatorMap[i]->key, mediatorName) == 0) {
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
        if (strcmp(this->mediatorMap[i]->key, mediatorName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->mediatorMapMutex);
    return exists;
}

static bool removeMediator(struct IView *self, const char *mediatorName, struct IMediator **mediator) {
    struct View *this = (struct View *) self;
    bool removed = false;

    mutex_lock(&this->mediatorMapMutex);

    size_t index = 0, i = 0;
    for (; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find mediator
        if (strcmp(this->mediatorMap[i]->key, mediatorName) == 0) { // match
            if (mediator != NULL) // out param
                *mediator = this->mediatorMap[i]->mediator;

            const char **interests = this->mediatorMap[i]->mediator->listNotificationInterests(this->mediatorMap[i]->mediator);
            for (const char **cursor = interests; *cursor != NULL; cursor++) { // remove notification observers
                self->removeObserver(self, *cursor, this->mediatorMap[i]->mediator);
            }
            this->mediatorMap[i]->mediator->onRemove(this->mediatorMap[i]->mediator);

            memset(&this->mediatorMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i) { // shift mediatorMap left
                const struct IMediator *previous = this->mediatorMap[i]->mediator;

                *this->mediatorMap[index] = *this->mediatorMap[i]; // shift left first
                memset(&this->mediatorMap[i]->key, 0, KEY_SIZE); // remove

                const char **interests = this->mediatorMap[index]->mediator->listNotificationInterests(this->mediatorMap[index]->mediator);
                for (const char **cursor = interests; *cursor; cursor++) { // update observer context to relocated mediators
                    for (size_t j = 0; this->observerMap[j] != NULL && this->observerMap[j]->key[0] != '\0'; j++) {
                        if (strcmp(this->observerMap[j]->key, *cursor) == 0) {
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

static void puremvc_view_init(struct IView *view, const char *key) {
    struct View *this = (struct View *) view;

    view->initializeView = initializeView;
    view->registerObserver = registerObserver;
    view->notifyObservers = notifyObservers;
    view->removeObserver = removeObserver;
    view->registerMediator = registerMediator;
    view->retrieveMediator = retrieveMediator;
    view->hasMediator = hasMediator;
    view->removeMediator = removeMediator;

    snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    mutex_init(&this->observerMapMutex);
    mutex_init(&this->mediatorMapMutex);
}

static void dispatchOnce(void) {
     mutex_init(&viewMapMutex);
}

struct IView *puremvc_view_getInstance(struct ViewMap **viewMap, const char *key) {
    if (viewMap == NULL && s_viewMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Missing ViewMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::View::getInstance] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return NULL;
    }

    if (s_viewMap == NULL)
        s_viewMap = viewMap;

    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t i = 0;
    for (; s_viewMap[i] != NULL && s_viewMap[i]->key[0] != '\0'; i++) { // find view
        if (strncmp(s_viewMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&viewMapMutex);
            return s_viewMap[i]->view;
        }
    }

    if (s_viewMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: ViewMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&viewMapMutex);
        return NULL;
    }

    if (s_viewMap[i]->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Missing View storage; skipping registration.\033[0m\n");
        return NULL;
    }

    snprintf(s_viewMap[i]->key, KEY_SIZE, "%s", key); // init
    puremvc_view_init(s_viewMap[i]->view, key);
    s_viewMap[i]->view->initializeView(s_viewMap[i]->view);

    mutex_unlock(&viewMapMutex);
    return s_viewMap[i]->view;
}

bool puremvc_view_removeView(const char *key, struct IView **view) {
    if (s_viewMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::removeView] FATAL: Missing ViewMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::removeView] FATAL: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::View::removeView] Error: Key '%s' too long (max %d) — skipping removal.\n", key, KEY_SIZE);
        return false;
    }

    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t index = 0;
    for (size_t i = 0; s_viewMap[i] != NULL && s_viewMap[i]->key[0] != '\0'; i++) { // find view
        if (strncmp(s_viewMap[i]->key, key, KEY_SIZE) == 0) {
            memset(&s_viewMap[i]->key, 0, KEY_SIZE); // remove
            if (view != NULL)
                *view = s_viewMap[i]->view;
        } else {
            if (index != i) { // shift left
                *s_viewMap[index] = *s_viewMap[i];
                memset(s_viewMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }

    if (index == 0) // all keys were removed; reset
        s_viewMap = NULL;

    mutex_unlock(&viewMapMutex);

    return true;
}
