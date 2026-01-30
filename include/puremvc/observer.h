/**
* @file observer.h
* @brief Observer Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include <stdbool.h>

#include "notification.h"

struct Observer {
    void *context;

    void (*notify)(const void *context, struct Notification notification);
    
    void *(*getContext)(const struct Observer *observer);
    
    void (*setContext)(struct Observer *observer, void *notifyContext);
    
    void (*(*getNotify)(const struct Observer *self))(const void *context, struct Notification notification);
    
    void (*setNotify)(struct Observer *observer, void (*notify)(const void *context, struct Notification notification));
    
    void (*notifyObserver)(const struct Observer *observer, struct Notification notification);
    
    bool (*compareNotifyContext)(const struct Observer *observer, const void *context);
};

struct Observer puremvc_observer(void (*notify)(const void *context, struct Notification notification), void *context);
