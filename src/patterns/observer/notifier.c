/**
* @file notifier.c
* @internal
* @brief Notifier Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/notifier.h"
// #include "puremvc/facade.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// static struct IFacade *getFacade(const struct INotifier *self) {
//     struct IFacade *facade = puremvc_facade_getInstance(self->getMultitonKey(self), puremvc_facade);
//     facade->initializeFacade(facade);
//     return facade;
// }

const char *getMultitonKey(const struct INotifier *self) {
    const struct Notifier *this = (struct Notifier *) self;
    if (this->key[0] == '\0') {
        fprintf(stderr, "\033[0;31m[PureMVC::Notifier::getMultitonKey] FATAL: multitonKey for this Notifier not yet initialized!.\033[0m\n");
        abort();
    }
    return this->key;
}

static void initializeNotifier(struct INotifier *self, const char *key) {
    struct Notifier *this = (struct Notifier *) self;
    int len = snprintf(this->key, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "\033[0;31m[PureMVC::Notifier::initializeNotifier] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d).\033[0m\n", key, len, KEY_SIZE);
}

// static void sendNotification(const struct INotifier *self, const char *notificationName, void *body, const char *type) {
//     const struct IFacade *facade = self->getFacade(self);
//     facade->sendNotification(facade, notificationName, body, type);
// }

struct INotifier *puremvc_notifier_init(struct Notifier *const notifier) {
    // notifier->base.getFacade = getFacade;
    notifier->base.getMultitonKey = getMultitonKey;
    notifier->base.initializeNotifier = initializeNotifier;
    // notifier->base.sendNotification = sendNotification;

    return &notifier->base;
}
