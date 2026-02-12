/**
* @file notifier.c
* @internal
* @brief Notifier Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "notifier.h"
#include "puremvc/i_facade.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct IFacade *getFacade(const struct INotifier *self) {
    struct IFacade *facade = puremvc_facade_getInstance(NULL, self->getMultitonKey(self));
    facade->initializeFacade(facade, NULL, NULL, NULL);
    return facade;
}

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
    this->key = key;
}

static void sendNotification(const struct INotifier *self, const char *notificationName, void *body, const char *type) {
    const struct IFacade *facade = self->getFacade(self);
    facade->sendNotification(facade, notificationName, body, type);
}

size_t puremvc_notifier_size() {
    return (sizeof(struct Notifier) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct INotifier *puremvc_notifier_init(void *buffer) {
    struct Notifier *this = (struct Notifier *) buffer;

    memset(this, 0, sizeof(struct Notifier));

    this->base.getFacade = getFacade;
    this->base.getMultitonKey = getMultitonKey;
    this->base.initializeNotifier = initializeNotifier;
    this->base.sendNotification = sendNotification;

    return (struct INotifier *) this;
}

