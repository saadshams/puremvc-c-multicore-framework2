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
    struct IObserver super;

    void *context;

    void (*notify)(const void *context, const struct INotification *notification);
};
