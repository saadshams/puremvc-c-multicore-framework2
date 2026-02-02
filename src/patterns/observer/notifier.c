/**
* @file notifier.c
* @internal
* @brief Notifier Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>
#include <string.h>

#include "puremvc/notifier.h"
#include "puremvc/facade.h"

static struct IFacade *getFacade(const struct INotifier *self) {
    struct IFacade *facade = puremvc_facade_getInstance(self->getMultitonKey(self), puremvc_facade);
    facade->initializeFacade(facade);
    return facade;
}

const char *getMultitonKey(const struct INotifier *self) {
    const struct Notifier *this = (struct Notifier *) self;
    return this->key;
}

static void initializeNotifier(struct INotifier *self, const char *key) {
    struct Notifier *this = (struct Notifier *) self;
    int len = snprintf(this->key, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Notifier::initializeNotifier] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);
}

static void sendNotification(const struct INotifier *self, const char *notificationName, void *body, const char *type) {
    const struct IFacade *facade = self->getFacade(self);
    facade->sendNotification(facade, notificationName, body, type);
}

struct Notifier puremvc_notifier() {
    return (struct Notifier) {
        .base = (struct INotifier) {
            .getFacade = getFacade,
            .getMultitonKey = getMultitonKey,
            .initializeNotifier = initializeNotifier,
            .sendNotification = sendNotification
        }
    };
}
