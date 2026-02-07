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
#include "i_mediator.h"
#include "i_observer.h"

struct View {
    struct IView base;

    char multitonKey[KEY_SIZE];

    Mutex mediatorMapMutex;
    struct MediatorMap {
        char key[KEY_SIZE];
        struct IMediator *mediator;
    } **mediatorMap;

    Mutex observerMapMutex;
    struct ObserverMap {
        char key[KEY_SIZE];
        struct IObserver **observers;
    } **observerMap;
};

struct ViewMap {
    char key[KEY_SIZE];
    struct View view;
};

struct IView *puremvc_view_getInstance(struct ViewMap **viewMap, const char *key);

void puremvc_view_removeView(struct ViewMap **viewMap, const char *key);
