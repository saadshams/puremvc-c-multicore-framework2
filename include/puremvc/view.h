/**
* @file view.h
* @brief View Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "mutex.h"
#include "constants.h"
#include "i_view.h"
#include "mediator.h"
#include "observer.h"

struct View {
    struct IView base;

    char multitonKey[KEY_SIZE];

    Mutex mediatorMapMutex;
    struct MediatorMap {
        char key[KEY_SIZE];
        struct Mediator mediator;
    } mediatorMap[MEDIATOR_MAP_SIZE];

    Mutex observerMapMutex;
    struct ObserverMap {
        char key[KEY_SIZE];
        struct Observer observers[OBSERVER_ARRAY_SIZE];
    } observerMap[OBSERVER_MAP_SIZE];
};

struct View puremvc_view(const char *key);

struct IView *puremvc_view_getInstance(const char *key, struct View(*factory)(const char *key));

void puremvc_view_removeView(const char *key);
