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
// #include "puremvc/IFacade.h"

// static struct IFacade *getFacade(const struct Notifier *self, const char **error) {
//     const struct Notifier *this = (struct Notifier *) self;
//     struct IFacade *facade = puremvc_facade_getInstance(this->key, puremvc_facade_new, error);
//     if (*error != NULL) return NULL;
//
//     facade->initializeFacade(facade, error);
//     if (*error != NULL) return NULL;
//
//     return facade;
// }

const char *getMultitonKey(const struct Notifier *self) {
    return self->key;
}

static void initializeNotifier(struct Notifier *self, const char *key) {
    snprintf(self->key, KEY_SIZE, "%s", key);
}

static void sendNotification(const struct Notifier *self, const char *notificationName, void *body, const char *type) {
    // const struct IFacade *facade = self->getFacade(self, error);
    // facade->sendNotification(facade, notificationName, body, type, error);
}

struct Notifier puremvc_notifier() {
    struct Notifier notifier = {0};

    // notifier.getFacade = getFacade;
    notifier.getMultitonKey = getMultitonKey;
    notifier.initializeNotifier = initializeNotifier;
    notifier.sendNotification = sendNotification;

    return notifier;
}
