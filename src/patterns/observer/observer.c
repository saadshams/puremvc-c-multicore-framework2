#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "observer.h"

static void *getNotifyContext(struct Observer *self) {
    return self->context;
}

static void setNotifyContext(struct Observer *self, void *notifyContext) {
    self->context = notifyContext;
}

static void (*getNotifyMethod(struct Observer *self))(void *context, struct Notification *notification) {
    return self->notify;
}

static void setNotifyMethod(struct Observer *self, void (*notifyMethod)(void *context, struct Notification *notification)) {
    self->notify = notifyMethod;
}

static void notifyObserver(struct Observer *self, struct Notification *notification) {
    self->notify(self->context, notification);
}

static bool compareNotifyContext(struct Observer *self, const void *context) {
    return self->context == context;
}

struct Observer *puremvc_observer_init(struct Observer *self) {
    self->getNotifyContext = getNotifyContext;
    self->setNotifyContext = setNotifyContext;
    self->getNotifyMethod = getNotifyMethod;
    self->setNotifyMethod = setNotifyMethod;
    self->notifyObserver = notifyObserver;
    self->compareNotifyContext = compareNotifyContext;
    return self;
}

struct Observer *puremvc_observer_alloc(void (*notifyMethod)(void *context, struct Notification *notification), void *notifyContext) {
    struct Observer *self = malloc(sizeof(struct Observer));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    self->notify = notifyMethod;
    self->context = notifyContext;
    puremvc_observer_init(self);
    return self;

    exception:
    fprintf(stderr, "Observer allocation failed.\n");
    return NULL;
}

struct Observer *puremvc_observer_new(void (*notifyMethod)(void *context, struct Notification *notification), void *notifyContext) {
    return puremvc_observer_init(puremvc_observer_alloc(notifyMethod, notifyContext));
}

void puremvc_observer_free(struct Observer **self) {
    (*self)->notify = NULL;
    (*self)->context = NULL;
    free(*self);
    *self = NULL;
}
