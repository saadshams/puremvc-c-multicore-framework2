/**
* @file observer.c
* @internal
* @brief Observer Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>

#include "puremvc/observer.h"

static void *getContext(const struct Observer *self) {
    return self->context;
}

static void setContext(struct Observer *self, void *notifyContext) {
    self->context = notifyContext;
}

static void (*getNotify(const struct Observer *self))(const void *context, struct Notification notification) {
    return self->notify;
}

static void setNotify(struct Observer *self, void (*notify)(const void *context, struct Notification notification)) {
    self->notify = notify;
}

static void notifyObserver(const struct Observer *self, struct Notification notification) {
    if (self->notify == NULL && self->context == NULL) return;
    self->notify(self->context, notification);
}

static bool compareNotifyContext(const struct Observer *self, const void *context) {
    if (self->context == NULL || context == NULL) return false;
    return self->context == context;
}

struct Observer puremvc_observer(void (*notify)(const void *context, struct Notification notification), void *context) {
    return (struct Observer) {
        .notify = notify,
        .context = context,
        .getContext = getContext,
        .setContext = setContext,
        .getNotify = getNotify,
        .setNotify = setNotify,
        .notifyObserver = notifyObserver,
        .compareNotifyContext = compareNotifyContext
    };
}
