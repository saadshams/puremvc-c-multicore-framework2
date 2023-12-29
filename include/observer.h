#ifndef PUREMVC_OBSERVER_H
#define PUREMVC_OBSERVER_H

#include "notification.h"
#include <stdbool.h>

struct Observer {
    void *context;
    void (*notify)(void *context, struct Notification *notification);

    void *(*getNotifyContext)(struct Observer *observer);
    void (*setNotifyContext)(struct Observer *observer, void *notifyContext);

    void (*(*getNotifyMethod)(struct Observer *self))(void *context, struct Notification *notification);
    void (*setNotifyMethod)(struct Observer *observer, void (*notifyMethod)(void *context, struct Notification *notification));

    void (*notifyObserver)(struct Observer *observer, struct Notification *notification);
    bool (*compareNotifyContext)(struct Observer *observer, const void *context);
};

struct Observer *puremvc_observer_init(struct Observer *self);

struct Observer *puremvc_observer_alloc(void (*notifyMethod)(void *context, struct Notification *notification), void *notifyContext);

struct Observer *puremvc_observer_new(void (*notifyMethod)(void *context, struct Notification *notification), void *notifyContext);

void puremvc_observer_free(struct Observer **self);

#endif //PUREMVC_OBSERVER_H
