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

static struct Facade *getFacade(const struct Notifier *self) {
    struct Facade *facade = puremvc_facade_getInstance(self->key, puremvc_facade);
    facade->initializeFacade(facade);
    return facade;
}

const char *getMultitonKey(const struct Notifier *self) {
    return self->key;
}

static void initializeNotifier(struct Notifier *self, const char *key) {
    snprintf(self->key, KEY_SIZE, "%s", key);
}

static void sendNotification(const struct Notifier *self, const char *notificationName, void *body, const char *type) {
    const struct Facade *facade = self->getFacade(self);
    facade->sendNotification(facade, notificationName, body, type);
}

struct Notifier puremvc_notifier() {
    struct Notifier notifier = {0};

    notifier.getFacade = getFacade;
    notifier.getMultitonKey = getMultitonKey;
    notifier.initializeNotifier = initializeNotifier;
    notifier.sendNotification = sendNotification;

    return notifier;
}
