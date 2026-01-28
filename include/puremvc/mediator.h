/**
* @file mediator.h
* @brief Mediator Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"
#include "notifier.h"
#include "notification.h"

#define MEDIATOR_NAME "mediator"

struct Mediator {
    char name[MAX_NAME_LEN];
    void *component;

    struct Notifier notifier;
    
    const char *(*getName)(const struct Mediator *self);
    void (*setComponent)(struct Mediator *self, void *component);
    void *(*getComponent)(const struct Mediator *self);

    const char **(*listNotificationInterests)(const struct Mediator *self, const char **error);
    void (*handleNotification)(const struct Mediator *self, struct Notification *notification);

    void (*onRegister)(struct Mediator *self);
    void (*onRemove)(struct Mediator *self);
};

struct Mediator puremvc_mediator(const char *name, void *component);
