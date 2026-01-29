/**
* @file notification.h
* @brief Notification Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include <stddef.h>

#include "puremvc/constants.h"

struct Notification {
    char name[NAME_SIZE];
    void *body;
    char type[NAME_SIZE];
    
    const char *(*getName)(const struct Notification *self);

    const void *(*getBody)(const struct Notification *self);

    void (*setBody)(struct Notification *self, void *body);

    const char *(*getType)(const struct Notification *self);

    void (*setType)(struct Notification *self, const char *type);

    void (*toString)(const struct Notification *self, char *buffer, size_t buffer_size);
};

struct Notification puremvc_notification(const char *name, void *body, const char *type);
