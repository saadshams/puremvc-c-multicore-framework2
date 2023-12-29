#ifndef PUREMVC_MEDIATOR_H
#define PUREMVC_MEDIATOR_H

#include "notification.h"

#define MEDIATOR_NAME "mediator"

struct Notifier;

struct Mediator {
    struct Notifier *notifier;

    const char *mediatorName;
    void *viewComponent;

    const char *(*getMediatorName)(struct Mediator *self);
    void (*setViewComponent)(struct Mediator *self, void *viewComponent);
    void *(*getViewComponent)(struct Mediator *self);

    const char **(*listNotificationInterests)(struct Mediator *self);
    void (*handleNotification)(struct Mediator *self, struct Notification *notification);
    void (*onRegister)(struct Mediator *self);
    void (*onRemove)(struct Mediator *self);
};

struct Mediator *puremvc_mediator_init(struct Mediator *self);

struct Mediator *puremvc_mediator_alloc(const char *mediatorName, void *viewComponent);

struct Mediator *puremvc_mediator_new(const char *mediatorName, void *viewComponent);

void puremvc_mediator_free(struct Mediator **self);

#endif //PUREMVC_MEDIATOR_H
