/**
* @file view.h
* @brief View Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_mutex.h"
#include "puremvc/i_view.h"

struct View {
    struct IView base; // headers

    const char *multitonKey; // pointers
    struct ObserverMap **observerMap;
    struct MediatorMap **mediatorMap;

    Mutex observerMapMutex; // payload
    Mutex mediatorMapMutex;
};
