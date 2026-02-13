/**
* @file view.h
* @brief View Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/platform.h"
#include "puremvc/i_view.h"

struct View {
    struct IView base; // headers

    char multitonKey[KEY_SIZE];  // pointers
    struct ObserverMap **observerMap;
    struct MediatorMap **mediatorMap;

    Mutex observerMapMutex; // payload
    Mutex mediatorMapMutex;
};
