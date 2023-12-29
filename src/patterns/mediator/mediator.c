#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mediator.h"
#include "notifier.h"

static const char *getMediatorName(struct Mediator *self) {
    return self->mediatorName;
}

static void setViewComponent(struct Mediator *self, void *viewComponent) {
    self->viewComponent = viewComponent;
}

static void *getViewComponent(struct Mediator *self) {
    return self->viewComponent;
}

static const char **listNotificationInterests(struct Mediator *self) {
    const char **interests = malloc(sizeof(char *));
    interests[0] = NULL;
    return interests;
}

static void handleNotification(struct Mediator *self, struct Notification *notification) {

}

static void onRegister(struct Mediator *self) {

}

static void onRemove(struct Mediator *self) {

}

struct Mediator *puremvc_mediator_init(struct Mediator *self) {
    self->notifier = puremvc_notifier_new();
    self->getMediatorName = getMediatorName;
    self->setViewComponent = setViewComponent;
    self->getViewComponent = getViewComponent;
    self->listNotificationInterests = listNotificationInterests;
    self->handleNotification = handleNotification;
    self->onRegister = onRegister;
    self->onRemove = onRemove;
    return self;
}

struct Mediator *puremvc_mediator_alloc(const char *mediatorName, void *viewComponent) {
    struct Mediator *self = malloc(sizeof(struct Mediator));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    self->mediatorName = mediatorName != NULL ? strdup(mediatorName) : strdup(MEDIATOR_NAME);
    if (self->mediatorName == NULL) {
        free(self);
        goto exception;
    }
    self->viewComponent = viewComponent;
    return self;

    exception:
    fprintf(stderr, "Mediator allocation failed.\n");
    return NULL;
}

struct Mediator *puremvc_mediator_new(const char *mediatorName, void *viewComponent) {
    return puremvc_mediator_init(puremvc_mediator_alloc(mediatorName, viewComponent));
}

void puremvc_mediator_free(struct Mediator **self) {
    puremvc_notifier_free(&(*self)->notifier);
    free((void *) (*self)->mediatorName);
    (*self)->mediatorName = NULL;
    (*self)->viewComponent = NULL;
    free(*self);
    *self = NULL;
}
