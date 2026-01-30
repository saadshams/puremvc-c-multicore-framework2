/**
* @file mediator.c
* @internal
* @brief Mediator Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>

#include "puremvc/mediator.h"

static const char *getName(const struct Mediator *self) {
    return self->name;
}

static void setComponent(struct Mediator *self, void *component) {
    self->component = component;
}

static void *getComponent(const struct Mediator *self) {
    return self->component;
}

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = { NULL };
    return interests;
}

static void handleNotification(const struct Mediator *self, struct Notification notification) {

}

static void onRegister(struct Mediator *self) {

}

static void onRemove(struct Mediator *self) {

}

struct Mediator puremvc_mediator(const char *name, void *component) {
    struct Mediator mediator = {0};

    snprintf(mediator.name, NAME_SIZE, "%s", name ? name : MEDIATOR_NAME);
    mediator.component = component;

    mediator.notifier = puremvc_notifier();
    
    mediator.getName = getName;
    mediator.setComponent = setComponent;
    mediator.getComponent = getComponent;
    mediator.listNotificationInterests = listNotificationInterests;
    mediator.handleNotification = handleNotification;
    mediator.onRegister = onRegister;
    mediator.onRemove = onRemove;

    return mediator;
}
