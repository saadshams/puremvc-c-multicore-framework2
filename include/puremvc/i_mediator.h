/**
 * @file i_mediator.h
 * @ingroup PureMVC
 * @brief IMediator Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "i_notifier.h"
#include "i_notification.h"

#define MEDIATOR_NAME "mediator"

/**
 * @struct IMediator
 * @brief Represents a mediator in the PureMVC framework.
 */
struct IMediator {
    const char *(*getName)(const struct IMediator *self);
    void (*setComponent)(struct IMediator *self, void *component);
    void *(*getComponent)(const struct IMediator *self);
    struct INotifier *(*getNotifier)(const struct IMediator *self);

    const char **(*listNotificationInterests)(const struct IMediator *self);
    void (*handleNotification)(const struct IMediator *self, struct INotification *notification);

    void (*onRegister)(struct IMediator *self);
    void (*onRemove)(struct IMediator *self);
};

size_t puremvc_mediator_size();

struct IMediator *puremvc_mediator_init(void *buffer, const char *name, void *component);
