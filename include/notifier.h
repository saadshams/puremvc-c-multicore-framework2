#ifndef PUREMVC_NOTIFIER_H
#define PUREMVC_NOTIFIER_H

#include "facade.h"

struct Notifier {

    // The Multiton Key for this app
    const char *multitonKey;

    /** Return the Multiton Facade instance */
    struct Facade *(*getFacade)(struct Notifier *self);

    void (*initializeNotifier)(struct Notifier *self, const char *key);

    void (*sendNotification)(struct Notifier *self, const char *notificationName, void *body, char *type);
};

struct Notifier *puremvc_notifier_init(struct Notifier *self);

struct Notifier *puremvc_notifier_alloc();

struct Notifier *puremvc_notifier_new();

void puremvc_notifier_free(struct Notifier **self);

#endif //PUREMVC_NOTIFIER_H
