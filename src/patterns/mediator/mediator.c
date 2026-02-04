/**
* @file mediator.c
* @internal
* @brief Mediator Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/mediator.h"

#include <stdio.h>
#include <string.h>

static const char *getName(const struct IMediator *self) {
    const struct Mediator *this = (struct Mediator *) self;
    return this->name;
}

static void setComponent(struct IMediator *self, void *component) {
    struct Mediator *this = (struct Mediator *) self;
    this->component = component;
}

static void *getComponent(const struct IMediator *self) {
    const struct Mediator *this = (struct Mediator *) self;
    return this->component;
}

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = { NULL };
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    (void)self; (void)notification;
}

static void onRegister(struct IMediator *self) {
    (void)self;
}

static void onRemove(struct IMediator *self) {
    (void)self;
}

struct Mediator puremvc_mediator(const char *name, void *component) {
    struct Mediator mediator = {
        .base = {
            .getName = getName,
            .setComponent = setComponent,
            .getComponent = getComponent,
            .listNotificationInterests = listNotificationInterests,
            .handleNotification = handleNotification,
            .onRegister = onRegister,
            .onRemove = onRemove
        },
        .component = component,
        .notifier = puremvc_notifier(),
    };

    int len = snprintf(mediator.name, NAME_SIZE, "%s", name ? name : MEDIATOR_NAME);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Mediator] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : MEDIATOR_NAME, len, NAME_SIZE);

    return mediator;
}

void puremvc_mediator_deinit(struct Mediator *mediator) {
    mediator->base = (struct IMediator){0};
    memset(&mediator->name, 0, KEY_SIZE);
    mediator->component = NULL;
}
