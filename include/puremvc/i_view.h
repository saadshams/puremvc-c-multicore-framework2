/**
 * @file i_view.h
 * @ingroup PureMVC
 * @brief IView Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include "constants.h"
#include "i_mediator.h"
#include "i_notification.h"

#include <stdbool.h>

struct ViewMap {
    char key[KEY_SIZE];
    struct IView *view;
};

/**
 * @struct IView
 * @brief Represents the View in the PureMVC framework.
 *
 * The View manages observers and mediators and is responsible
 * for delivering notifications to interested parties.
 */
struct IView {
    void (*initializeView)(struct IView *self);

    bool (*registerObserver)(struct IView *self, const char *notificationName, void (*notify)(const void *context, const struct INotification *notification), void *context);

    void (*notifyObservers)(const struct IView *self, const struct INotification *notification);

    bool (*removeObserver)(struct IView *self, const char *notificationName, const void *notifyContext);

    bool (*registerMediator)(struct IView *self, struct IMediator *(*factory)(struct IMediator *mediator, const char *name, void *component), const char *name, void *component);

    struct IMediator *(*retrieveMediator)(const struct IView *self, const char *mediatorName);

    bool (*hasMediator)(const struct IView *self, const char *mediatorName);

    bool (*removeMediator)(struct IView *self, const char *mediatorName, struct IMediator **mediator);
};

struct IView *puremvc_view_getInstance(struct ViewMap **viewMap, const char *key);

bool puremvc_view_removeView(const char *key, struct IView **view);
