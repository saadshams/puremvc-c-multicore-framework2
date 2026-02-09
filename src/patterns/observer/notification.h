/**
* @file notification.h
* @brief Notification Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_notification.h"

struct Notification {
    struct INotification base;

    const char *name;
    void *body;
    const char *type;
};
