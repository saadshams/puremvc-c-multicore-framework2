/**
* @file mediator.c
* @internal
* @brief Mediator Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/mediator.h"
#include "puremvc/notifier.h"

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

struct IMediator *puremvc_mediator(struct Mediator *const mediator, const char *name, void *component) {
    mediator->base.getName = getName;
    mediator->base.setComponent = setComponent;
    mediator->base.getComponent = getComponent;
    mediator->base.listNotificationInterests = listNotificationInterests;
    mediator->base.handleNotification = handleNotification;
    mediator->base.onRegister = onRegister;
    mediator->base.onRemove = onRemove;

    mediator->component = component;
    mediator->base.notifier = puremvc_notifier(&(struct Notifier){0});

    int len = snprintf(mediator->name, NAME_SIZE, "%s", name ? name : MEDIATOR_NAME);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Mediator] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : MEDIATOR_NAME, len, NAME_SIZE);

    return &mediator->base;
}

void puremvc_mediator_deinit(struct Mediator *mediator) {
    mediator->base = (struct IMediator){0};
    memset(&mediator->name, 0, KEY_SIZE);
    mediator->component = NULL;
}
