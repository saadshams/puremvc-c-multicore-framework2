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
