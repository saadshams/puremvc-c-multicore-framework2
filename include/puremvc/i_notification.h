/**
 * @file i_notification.h
 * @ingroup PureMVC
 * @brief INotification Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include <stddef.h>

/**
 * @struct INotification
 * @brief Represents a notification in the PureMVC framework.
 */
struct INotification {
    const char *(*getName)(const struct INotification *self);

    const void *(*getBody)(const struct INotification *self);

    void (*setBody)(struct INotification *self, void *body);

    const char *(*getType)(const struct INotification *self);

    void (*setType)(struct INotification *self, const char *type);

    void (*toString)(const struct INotification *self, char *buffer, size_t buffer_size);
};

size_t puremvc_notification_size(const char *name, const char *type);

struct INotification *puremvc_notification_init(void *buffer, const char *name, void *body, const char *type);
