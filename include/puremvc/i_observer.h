/**
 * @file i_observer.h
 * @ingroup PureMVC
 * @brief IObserver Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "i_notification.h"

#include <stdbool.h>

struct IObserver {
    void *(*getContext)(const struct IObserver *observer);

    void (*setContext)(struct IObserver *observer, void *notifyContext);

    bool (*(*getNotify)(const struct IObserver *self))(const void *context, const struct INotification *notification);

    void (*setNotify)(struct IObserver *observer, bool (*notify)(const void *context, const struct INotification *notification));

    void (*notifyObserver)(const struct IObserver *observer, const struct INotification *notification);

    bool (*compareNotifyContext)(const struct IObserver *observer, const void *context);
};

size_t puremvc_observer_size();

struct IObserver *puremvc_observer_init(void *buffer, bool (*notify)(const void *context, const struct INotification *notification), void *context);
