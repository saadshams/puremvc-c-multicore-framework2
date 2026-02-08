/**
* @file notification.h
* @brief Notification Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"
#include "i_notification.h"

struct Notification {
    struct INotification base;

    char name[NAME_SIZE];
    void *body;
    char type[NAME_SIZE];
};
