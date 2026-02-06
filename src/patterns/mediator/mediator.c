/**
* @file mediator.c
* @internal
* @brief Mediator Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/mediator.h"
#include "puremvc/i_notifier.h"

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

static struct INotifier *getNotifier(const struct IMediator *self) {
    const struct Mediator *this = (struct Mediator *) self;
    return &this->notifier.base;
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

struct IMediator *puremvc_mediator_init(struct IMediator *const mediator, const char *name, void *component) {
    struct Mediator *this = (struct Mediator *) mediator;

    mediator->getName = getName;
    mediator->setComponent = setComponent;
    mediator->getComponent = getComponent;
    mediator->getNotifier = getNotifier;
    mediator->listNotificationInterests = listNotificationInterests;
    mediator->handleNotification = handleNotification;
    mediator->onRegister = onRegister;
    mediator->onRemove = onRemove;

    int len = snprintf(this->name, NAME_SIZE, "%s", name ? name : MEDIATOR_NAME);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Mediator] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : MEDIATOR_NAME, len, NAME_SIZE);

    this->component = component;

    puremvc_notifier_init(&this->notifier);

    return mediator;
}
