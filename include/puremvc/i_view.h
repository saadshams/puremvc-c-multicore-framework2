/**
 * @file i_view.h
 * @ingroup PureMVC
 * @brief IView Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "i_notification.h"
#include "i_mediator.h"

#include <stdbool.h>

/**
 * @struct IView
 * @brief Represents the View in the PureMVC framework.
 *
 * The View manages observers and mediators and is responsible
 * for delivering notifications to interested parties.
 */
struct IView {
    void (*initializeView)(struct IView *self);

    void (*registerObserver)(const struct IView *self, const char *notificationName, void (*notify)(const void *context, struct INotification *notification), void *context);

    void (*notifyObservers)(const struct IView *self, struct INotification *notification);

    void (*removeObserver)(const struct IView *self, const char *notificationName, const void *notifyContext);

    void (*registerMediator)(const struct IView *self, struct IMediator *(*factory)(struct IMediator *mediator, const char *name, void *component), const char *name, void *component);

    struct IMediator *(*retrieveMediator)(const struct IView *self, const char *mediatorName);

    bool (*hasMediator)(const struct IView *self, const char *mediatorName);

    struct Mediator(*removeMediator)(const struct IView *self, const char *mediatorName);
};
