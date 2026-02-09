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

    void *body;

    size_t name_len;
    size_t type_len;
    char data[]; // Double-Buffer FAM Pattern: data = [name][\0][type][\0]
};
