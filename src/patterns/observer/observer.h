/**
* @file observer.h
* @brief Observer Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_observer.h"

struct Observer {
    struct IObserver base;

    void *context;

    bool (*notify)(const void *context, const struct INotification *notification);
};
