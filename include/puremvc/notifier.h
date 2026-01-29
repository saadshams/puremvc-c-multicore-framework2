/**
* @file notifier.h
* @brief Notifier Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"

struct Notifier {
    char key[NAME_SIZE];
    
    // struct IFacade *(*getFacade)(const struct Notifier *self);
    
    const char *(*getMultitonKey)(const struct Notifier *self);
    
    void (*initializeNotifier)(struct Notifier *self, const char *key);
    
    void (*sendNotification)(const struct Notifier *self, const char *notificationName, void *body, const char *type);
};

struct Notifier puremvc_notifier();
