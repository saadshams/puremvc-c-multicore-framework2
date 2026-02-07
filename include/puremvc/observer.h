/**
* @file observer.h
* @brief Observer Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "i_observer.h"
#include "i_notification.h"

struct Observer {
    struct IObserver base;

    void *context;

    void (*notify)(const void *context, const struct INotification *notification);
};

struct IObserver *puremvc_observer_init(struct Observer *observer, void (*notify)(const void *context, const struct INotification *notification), void *context);
