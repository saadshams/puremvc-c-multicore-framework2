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

// mutex for viewMap
static Mutex viewMapMutex;
static MutexOnce viewMutexOnce = MUTEX_ONCE_INIT;

static void initializeView(struct IView *self) {
    (void)self;
}

// api change since the search needs to happen on the user slots and instantiate it
static void registerObserver(const struct IView *self, const char *notificationName, void (*notify)(const void *context, struct INotification *notification), void *context) {
    struct View *this = (struct View *) self;

    if (strlen(notificationName) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Observer::registerObserver] Error: notification name '%s' too long (max %d) — skipping registration.\n", notificationName, KEY_SIZE);
        return;
    }

    if (this->observerMap == NULL) { // missing ObserverMap field
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] FATAL: Missing ObserverMap field in ViewMap; skipping registration.\033[0m\n");
        return;
    }

    mutex_lock(&this->observerMapMutex);

    size_t i = 0;
    for (; this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find existing
        if (strcmp(this->observerMap[i]->key, notificationName) != 0) // mismatch
            continue;

        struct IObserver **observers = this->observerMap[i]->observers;

        size_t j = 0; // find available observer slot
        for (; observers[j] != NULL && observers[j]->getContext != NULL && observers[j]->getContext(observers[j]) != NULL; j++) {}

        if (observers[j] == NULL) { // overflow (Observer)
            fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] ERROR: Observer storage overflow for notification '%s' at ObserverMap index %zu; increase slots - skipping registration.\033[0m\n", notificationName, i);
            mutex_unlock(&this->observerMapMutex);
            return;
        }

        puremvc_observer_init((struct Observer *) observers[j], notify, context); // registration (existing key)
        mutex_unlock(&this->observerMapMutex);
        return;
    }

    if (this->observerMap[i] == NULL) { // overflow (ObserverMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: ObserverMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->observerMapMutex);
        return;
    }

    if (this->observerMap[i]->observers == NULL || this->observerMap[i]->observers[0] == NULL) { // overflow (Observer)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerObserver] Error: Observer storage overflow for notification '%s' at ObserverMap index %zu; increase slots - skipping registration.\033[0m\n", notificationName, i);
        mutex_unlock(&this->observerMapMutex);
        return;
    }

    snprintf(this->observerMap[i]->key, KEY_SIZE, "%s", notificationName); // registration (new key)
    puremvc_observer_init((struct Observer *) this->observerMap[i]->observers[0], notify, context);
    mutex_unlock(&this->observerMapMutex);
}

static void notifyObservers(const struct IView *self, struct INotification *notification) {
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

void removeObserver(const struct IView *self, const char *notificationName, const void *notifyContext) {
    struct View *this = (struct View *) self;
    mutex_lock(&this->observerMapMutex);

    for (size_t i = 0; this->observerMap != NULL && this->observerMap[i] != NULL && this->observerMap[i]->key[0] != '\0'; i++) { // find observer
        if (strcmp(this->observerMap[i]->key, notificationName) == 0) { // match (observerMap key)
            struct IObserver **observers = this->observerMap[i]->observers;

            size_t index = 0, j = 0; // find observer to remove
            for (; observers != NULL && observers[j] != NULL && observers[j]->getContext(observers[j]) != NULL; j++) {
                const struct IObserver *observer = observers[j];
                if (observer->compareNotifyContext(observer, notifyContext) == false) { // mismatch (shift left to remove)
                    if (index != j) { // shift observers left
                        observers[index] = observers[j];
                    }
                    index++;
                }
            }

            for (size_t k = index; this->observerMap[i]->observers[k] != NULL; k++) { // reinit tail observers after shift left
                puremvc_observer_init((struct Observer *) this->observerMap[i]->observers[k], NULL, NULL);
            }

            if (index == 0) { // ObserverMap empty; shift left ObservrerMap
                memset(&this->observerMap[i]->key, 0, KEY_SIZE);

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
}

static void registerMediator(const struct IView *self, struct IMediator *(*factory)(struct IMediator *mediator, const char *name, void *component), const char *name, void *component) {
    struct View *this = (struct View *) self;

    if (strlen(name) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: Key '%s' too long (max %d) — skipping registration.\033[0m\n", name, KEY_SIZE);
        return;
    }

    if (this->mediatorMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] FATAL: Missing MediatorMap field in ViewMap; skipping registration.\033[0m\n");
        return;
    }

    mutex_lock(&this->mediatorMapMutex);

    size_t i = 0;
    for (; this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find existing, no override; return;
        if (strcmp(this->mediatorMap[i]->key, name) == 0) {
            fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: Mediator '%s' exists; skipping registration\033[0m.\n", name);
            mutex_unlock(&this->mediatorMapMutex);
            return;
        }
    }

    if (this->mediatorMap[i] == NULL) { // overflow (MediatorMap)
        fprintf(stderr, "\033[0;31m[PureMVC::View::registerMediator] Error: MediatorMap storage overflow for mediator '%s'; increase slots - skipping registration.\033[0m\n", name);
        mutex_unlock(&this->mediatorMapMutex);
        return;
    }

    // todo check if mediator exists
    struct IMediator *mediator = factory(this->mediatorMap[i]->mediator, name, component); // registration
    snprintf(this->mediatorMap[i]->key, KEY_SIZE, "%s", mediator->getName(mediator));

    mediator->getNotifier(mediator)->initializeNotifier(mediator->getNotifier(mediator), this->multitonKey);

    const char **interests = mediator->listNotificationInterests(mediator);
    for (const char **interest = interests; *interest; interest++) { // register observers (mutex guards context if mediator is removed)
        self->registerObserver(self, *interest, (void (*)(const void *, struct INotification *)) this->mediatorMap[i]->mediator->handleNotification, this->mediatorMap[i]->mediator); // pass slot, not the temp
    }

    mutex_unlock(&this->mediatorMapMutex);

    mediator->onRegister(mediator);
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

static struct Mediator removeMediator(const struct IView *self, const char *mediatorName) {
    struct View *this = (struct View *) self;
    mutex_lock(&this->mediatorMapMutex);
    struct Mediator mediator = {0};

    size_t index = 0, i = 0;
    for (; this->mediatorMap != NULL && this->mediatorMap[i] != NULL && this->mediatorMap[i]->key[0] != '\0'; i++) { // find mediator
        if (strcmp(this->mediatorMap[i]->key, mediatorName) == 0) { // match
            memcpy(&mediator, this->mediatorMap[i]->mediator, sizeof(struct Mediator));

            const char **interests = this->mediatorMap[i]->mediator->listNotificationInterests(this->mediatorMap[i]->mediator);
            for (const char **cursor = interests; *cursor != NULL; cursor++) { // remove interests
                this->base.removeObserver(self, *cursor, this->mediatorMap[i]->mediator);
            }
            this->mediatorMap[i]->mediator->onRemove(this->mediatorMap[i]->mediator);

            memset(&this->mediatorMap[i]->key, 0, KEY_SIZE);
            puremvc_mediator_init(this->mediatorMap[i]->mediator, NULL, NULL);
        } else {
            if (index != i) { // shift mediatorMap left
                const struct IMediator *previous = this->mediatorMap[i]->mediator;

                snprintf(this->mediatorMap[index]->key, KEY_SIZE, "%s", this->mediatorMap[i]->key);
                this->mediatorMap[index] = this->mediatorMap[i];

                memset(&this->mediatorMap[i]->key, 0, KEY_SIZE);
                puremvc_mediator_init(this->mediatorMap[i]->mediator, NULL, NULL);

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

    // j & k
    // mark the last NULL but it NULLS the last and the second last entry
    // if (index > 0 && this->observerMap[0]->observers[index]->setContext != NULL) {
        // this->observerMap[0]->observers[index]->setContext(this->observerMap[0]->observers[index], NULL);
        // puremvc_observer((struct Observer *) this->observerMap[0]->observers[index], NULL, NULL);
    // }


    mutex_unlock(&this->mediatorMapMutex);
    return mediator;
}

static void init(struct View *view, const char *key) {
    view->base = (struct IView) {
        .initializeView = initializeView,
        .registerObserver = registerObserver,
        .notifyObservers = notifyObservers,
        .removeObserver = removeObserver,
        .registerMediator = registerMediator,
        .retrieveMediator = retrieveMediator,
        .hasMediator = hasMediator,
        .removeMediator = removeMediator
    };

    snprintf(view->multitonKey, KEY_SIZE, "%s", key);
    mutex_init(&view->observerMapMutex);
    mutex_init(&view->mediatorMapMutex);
}

static void deinit(struct View *view) {
    memset(&view->multitonKey, 0, KEY_SIZE);
    view->base = (struct IView) {0};

    mutex_destroy(&view->observerMapMutex);
    mutex_destroy(&view->mediatorMapMutex);

    for (size_t i = 0; view->observerMap != NULL && view->observerMap[i] != NULL; i++) { // clear observerMap
        memset(view->observerMap[i]->key, 0, KEY_SIZE);
        for (size_t j = 0; view->observerMap[i]->observers != NULL && view->observerMap[i]->observers[j] != NULL; j++) {
            // puremvc_observer_deinit(view->observerMap[i]->observers[j]);
        }
    }

    for (size_t j = 0; view->mediatorMap != NULL && view->mediatorMap[j] != NULL; j++) { // clear mediatorMap
        memset(view->mediatorMap[j]->key, 0, KEY_SIZE);
        // puremvc_mediator_deinit(&view->mediatorMap[j]->mediator);
    }
}

static void dispatchOnce(void) {
     mutex_init(&viewMapMutex);
}

struct IView *puremvc_view_getInstance(struct ViewMap **viewMap, const char *key) {
    if (viewMap == NULL) {
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

    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t i = 0;
    for (; viewMap[i] != NULL && viewMap[i]->key[0] != '\0'; i++) { // find view
        if (strncmp(viewMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&viewMapMutex);
            return &viewMap[i]->view.base;
        }
    }

    if (viewMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: ViewMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&viewMapMutex);
        return NULL;
    }

    snprintf(viewMap[i]->key, KEY_SIZE, "%s", key); // init
    init(&viewMap[i]->view, key);
    viewMap[i]->view.base.initializeView(&viewMap[i]->view.base);

    mutex_unlock(&viewMapMutex);
    return &viewMap[i]->view.base;
}

void puremvc_view_removeView(struct ViewMap **viewMap, const char *key) {
    if (viewMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::removeView] FATAL: Missing ViewMap storage; skipping registration.\033[0m\n");
        return;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::removeView] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::View::removeView] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return;
    }

    mutex_once(&viewMutexOnce, dispatchOnce);
    mutex_lock(&viewMapMutex);

    size_t index = 0;
    for (size_t i = 0; viewMap[i] != NULL && viewMap[i]->key[0] != '\0'; i++) { // find view
        if (strncmp(viewMap[i]->key, key, KEY_SIZE) == 0) {
            memset(&viewMap[i]->key, 0, KEY_SIZE); // clear view
            deinit(&viewMap[i]->view);
        } else {
            if (index != i) { // shift left
                snprintf(viewMap[index]->key, KEY_SIZE, "%s", viewMap[i]->key); // copy view (destination)
                viewMap[index]->view = viewMap[i]->view;

                memset(viewMap[i]->key, 0, KEY_SIZE); // clear view (source)
                deinit(&viewMap[i]->view);
            }
            index++;
        }
    }
    mutex_unlock(&viewMapMutex);
}
