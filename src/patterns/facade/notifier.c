#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "notifier.h"

static struct Facade *getFacade(struct Notifier *self) {
    return puremvc_facade_getInstance(self->multitonKey, puremvc_facade_new);
}

static void sendNotification(struct Notifier *self, const char *notificationName, void *body, char *type) {
    struct Facade *facade = self->getFacade(self);
    facade->sendNotification(facade, notificationName, body, type);
}

static void initializeNotifier(struct Notifier *self, const char *key) {
    self->multitonKey = strdup(key);
}

struct Notifier *puremvc_notifier_init(struct Notifier *self) {
    self->getFacade = getFacade;
    self->initializeNotifier = initializeNotifier;
    self->sendNotification = sendNotification;
    return self;
}

struct Notifier *puremvc_notifier_alloc() {
    struct Notifier *self = malloc(sizeof(struct Notifier));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    puremvc_notifier_init(self);
    self->multitonKey = NULL;
    return self;

    exception:
    fprintf(stderr, "Notifier allocation failed.\n");
    return NULL;
}

struct Notifier *puremvc_notifier_new() {
    return puremvc_notifier_init(puremvc_notifier_alloc());
}

void puremvc_notifier_free(struct Notifier **self) {
    free((void *)(*self)->multitonKey);
    free(*self);
    *self = NULL;
}
