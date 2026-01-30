/**
* @file view.h
* @brief View Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "constants.h"
#include "mediator.h"
#include "observer.h"

struct View {
    char multitonKey[KEY_SIZE];

    // mutex for mediatorMap
    // Mutex mediatorMapMutex;
    struct MediatorMap {
        char key[KEY_SIZE];
        struct Mediator mediator;
    } mediatorMap[MEDIATORS_MAP_SIZE];
    size_t mediatorsMapCount;

    // mutex for observerMap
    // Mutex observerMapMutex;
    struct ObserverMap {
        char key[KEY_SIZE];
        struct Observer observers[OBSERVERS_ARRAY_SIZE];
        size_t observersCount;
    } observerMap[OBSERVER_MAP_SIZE];
    size_t observersMapCount;

    void (*initializeView)(struct View *self);

    void (*registerObserver)(struct View *self, const char *notificationName, struct Observer observer);

    void (*notifyObservers)(const struct View *self, struct Notification notification);

    void (*removeObserver)(struct View *self, const char *notificationName, const void *notifyContext);

    void (*registerMediator)(struct View *self, struct Mediator mediator);

    struct Mediator *(*retrieveMediator)(struct View *self, const char *mediatorName);

    bool (*hasMediator)(const struct View *self, const char *mediatorName);

    struct Mediator(*removeMediator)(struct View *self, const char *mediatorName);
};

struct View puremvc_view(const char *key);

struct View *puremvc_view_getInstance(const char *key, struct View(*factory)(const char *key));

void puremvc_view_removeView(const char *key);