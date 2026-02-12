/**
* @file mediator.c
* @internal
* @brief Mediator Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "mediator.h"

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
    struct Mediator *this = (struct Mediator *) self;
    return (struct INotifier *) &this->notifier;
}

static const char *const *listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *const interests[] = { NULL };
    return interests;
}

static bool handleNotification(const struct IMediator *self, struct INotification *notification) {
    (void)self; (void)notification;
    return true;
}

static void onRegister(struct IMediator *self) {
    (void)self;
}

static void onRemove(struct IMediator *self) {
    (void)self;
}

size_t puremvc_mediator_size() {
    return (sizeof(struct Mediator) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct IMediator *puremvc_mediator_init(void *buffer, const char *name, void *component) {
    struct Mediator *this = (struct Mediator *) buffer;

    memset(this, 0, sizeof(struct Mediator));

    this->base.getName = getName;
    this->base.setComponent = setComponent;
    this->base.getComponent = getComponent;
    this->base.getNotifier = getNotifier;
    this->base.listNotificationInterests = listNotificationInterests;
    this->base.handleNotification = handleNotification;
    this->base.onRegister = onRegister;
    this->base.onRemove = onRemove;

    this->name = name != NULL ? name : MEDIATOR_NAME;
    this->component = component;

    puremvc_notifier_init((struct INotifier *) &this->notifier);

    return (struct IMediator *) this;
}
