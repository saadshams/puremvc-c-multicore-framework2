#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "view.h"
#include "notifier.h"
#include "array.h"

// The Multiton View instanceMap.
static Dictionary *instanceMap;

// mutex for instanceMap
static pthread_rwlock_t instanceMap_mutex = PTHREAD_RWLOCK_INITIALIZER;

// mutex for mediatorMap
static pthread_rwlock_t mediatorMap_mutex = PTHREAD_RWLOCK_INITIALIZER;

// mutex for observerMap
static pthread_rwlock_t observerMap_mutex = PTHREAD_RWLOCK_INITIALIZER;

static void initializeView(struct View *self) {

}

static void registerObserver(struct View *self, const char *notificationName, struct Observer *observer) {
    pthread_rwlock_wrlock(&observerMap_mutex);
    if (self->observerMap->containsKey(self->observerMap, notificationName)) {
        Array *observers = (Array *) self->observerMap->get(self->observerMap, notificationName);
        observers->push(observers, observer);
    } else {
        Array *observers = puremvc_array_new();
        observers->push(observers, observer);
        self->observerMap->put(self->observerMap, notificationName, observers);
    }
    pthread_rwlock_unlock(&observerMap_mutex);
}

static void notifyObservers(struct View *self, struct Notification *notification) {
    pthread_rwlock_rdlock(&observerMap_mutex);

    if (self->observerMap->containsKey(self->observerMap, notification->name)) {
        // Get a reference to the clone list for this notification name
        Array *observers = (Array *) self->observerMap->get(self->observerMap, notification->name);

        // Copy observers from reference array to working array,
        // since the reference array may change during the notification loop
        Array *copy = observers->clone(observers);
        pthread_rwlock_unlock(&observerMap_mutex); // unlock - lock not required on the copy list

        // Notify Observers from the working array
        for (List *cursor = copy->list; cursor; cursor = cursor->next) {
            struct Observer *observer = (struct Observer *) cursor->item;
            observer->notifyObserver(observer, notification);
        }

        copy->clear(copy, NULL);
        puremvc_array_free(&copy);
    }
    pthread_rwlock_unlock(&observerMap_mutex);
}

static void removeObserver(struct View *self, const char *notificationName, void *notifyContext) {
    pthread_rwlock_wrlock(&observerMap_mutex);

    Array *array = (Array *) self->observerMap->get(self->observerMap, notificationName);

    for (List *cursor = array->list; cursor; cursor = cursor->next) {
        struct Observer *observer = (struct Observer *) cursor->item;
        if (observer->compareNotifyContext(observer, notifyContext)) {
            array->removeItem(array, observer);
            puremvc_observer_free(&observer);
            break;
        }
    }

    if (array->size(array) == 0) {
        // free array observers value of the key
        self->observerMap->removeItem(self->observerMap, notificationName);
    }

    pthread_rwlock_unlock(&observerMap_mutex);
}

static void registerMediator(struct View *self, struct Mediator *mediator) {
    pthread_rwlock_wrlock(&mediatorMap_mutex);

    // do not allow re-registration (you must to removeMediator first)
    if (self->mediatorMap->containsKey(self->mediatorMap, mediator->mediatorName)) {
        pthread_rwlock_unlock(&mediatorMap_mutex);
        return;
    }

    mediator->notifier->initializeNotifier(mediator->notifier, self->multitonKey);

    self->mediatorMap->put(self->mediatorMap, mediator->mediatorName, mediator);

    // Get Notification interests, if any.
    const char **interests = mediator->listNotificationInterests(mediator);

    // Register Mediator as an observer for each notification of interests
    for(; *interests; interests++) {
        // Create Observer referencing this mediator's handleNotification method
        struct Observer *observer = puremvc_observer_new((void (*)(void *, struct Notification *)) mediator->handleNotification, mediator);

        // Register Mediator as Observer for its list of Notification interests
        self->registerObserver(self, *interests, observer);
    }

    // alert the mediator that it has been registered
    mediator->onRegister(mediator);

    free((void *) *interests);

    pthread_rwlock_unlock(&mediatorMap_mutex);
}

static struct Mediator *retrieveMediator(struct View *self, const char *mediatorName) {
    pthread_rwlock_rdlock(&mediatorMap_mutex);
    struct Mediator *mediator = (struct Mediator *) self->mediatorMap->get(self->mediatorMap, mediatorName);
    pthread_rwlock_unlock(&mediatorMap_mutex);
    return mediator;
}

static bool hasMediator(struct View *self, const char *mediatorName) {
    pthread_rwlock_rdlock(&mediatorMap_mutex);
    bool exists = self->mediatorMap->containsKey(self->mediatorMap, mediatorName);
    pthread_rwlock_unlock(&mediatorMap_mutex);
    return exists;
}

static struct Mediator *removeMediator(struct View *self, const char *mediatorName) {
    pthread_rwlock_wrlock(&mediatorMap_mutex);
    struct Mediator *mediator = (struct Mediator *) self->mediatorMap->removeItem(self->mediatorMap, mediatorName);
    if (mediator) {
        // for every notification this mediator is interested in...
        const char *const *interests = mediator->listNotificationInterests(mediator);
        for (; *interests; interests++) {
            // remove the observer linking the mediator
            // to the notification interest
            self->removeObserver(self, *interests, mediator);
        }

        // alert the mediator that it has been removed
        mediator->onRemove(mediator);
        pthread_rwlock_unlock(&mediatorMap_mutex);
        return mediator;
    }
    pthread_rwlock_unlock(&mediatorMap_mutex);
    return NULL;
}

struct View *puremvc_view_init(struct View *self) {
    self->initializeView = initializeView;
    self->registerObserver = registerObserver;
    self->notifyObservers = notifyObservers;
    self->removeObserver = removeObserver;
    self->registerMediator = registerMediator;
    self->retrieveMediator = retrieveMediator;
    self->hasMediator = hasMediator;
    self->removeMediator = removeMediator;
    return self;
}

struct View *puremvc_view_alloc(const char *key) {
    assert(instanceMap->get(instanceMap, key) == NULL);

    struct View *self = malloc(sizeof(struct View));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    puremvc_view_init(self);
    self->multitonKey = strdup(key);
    self->mediatorMap = puremvc_dictionary_new();
    self->observerMap = puremvc_dictionary_new();
    return self;

    exception:
    fprintf(stderr, "View allocation failed.\n");
    return NULL;
}

struct View *puremvc_view_new(const char *key) {
    return puremvc_view_init(puremvc_view_alloc(key));
}

void puremvc_view_free(struct View **self) {
    free((void *) (*self)->multitonKey);
    (*self)->mediatorMap->clear((*self)->mediatorMap, free);
    puremvc_dictionary_free(&(*self)->mediatorMap);
    (*self)->observerMap->clear((*self)->observerMap, free);
    puremvc_dictionary_free(&(*self)->observerMap);

    free(*self);
    *self = NULL;
}

struct View *puremvc_view_getInstance(const char *key, struct View *(*factory)(const char *)) {
    pthread_rwlock_wrlock(&instanceMap_mutex);

    if (instanceMap == NULL) instanceMap = puremvc_dictionary_new();

    struct View *instance = (struct View *) instanceMap->get(instanceMap, key);
    if (instance == NULL) {
        instance = (struct View *) instanceMap->put(instanceMap, key, factory(key));
        instance->initializeView(instance);
    }
    pthread_rwlock_unlock(&instanceMap_mutex);
    return instance;
}

void puremvc_view_removeView(const char *key) {
    pthread_rwlock_wrlock(&instanceMap_mutex);
    struct View *view = (struct View *) instanceMap->removeItem(instanceMap, key);
    if (view != NULL) puremvc_view_free(&view);
    pthread_rwlock_unlock(&instanceMap_mutex);
}
