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
    const struct Mediator *this = (const struct Mediator *) self;
    return this->name;
}

static void setComponent(struct IMediator *self, void *component) {
    struct Mediator *this = (struct Mediator *) self;
    this->component = component;
}

static void *getComponent(const struct IMediator *self) {
    const struct Mediator *this = (const struct Mediator *) self;
    return this->component;
}

static struct INotifier *getNotifier(const struct IMediator *self) {
    const struct Mediator *this = (const struct Mediator *) self;
    return (struct INotifier *) &this->notifier;
}

static const char *const *listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *const interests[] = { NULL };
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    (void)self;
    (void)notification;
}

static void onRegister(struct IMediator *self) {
    (void)self;
}

static void onRemove(struct IMediator *self) {
    (void)self;
}

size_t puremvc_mediator_size(void) {
    return (sizeof(struct Mediator) + (sizeof(void *) - 1u)) & ~(sizeof(void *) - 1u);
}

struct IMediator *puremvc_mediator_init(void *buffer, const char *name, void *component) {
    if (buffer == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Mediator::init] Error: Buffer is NULL for mediator '%s' - skipping initialization.\033[0m\n", name != NULL ? name : "(unnamed)");
        return NULL;
    }

    struct Mediator *this = (struct Mediator *) buffer;
    memset(this, 0, sizeof(struct Mediator));

    this->super.getName = getName;
    this->super.setComponent = setComponent;
    this->super.getComponent = getComponent;
    this->super.getNotifier = getNotifier;
    this->super.listNotificationInterests = listNotificationInterests;
    this->super.handleNotification = handleNotification;
    this->super.onRegister = onRegister;
    this->super.onRemove = onRemove;

    const char *key = name != NULL ? name : MEDIATOR_NAME;
    int len = snprintf(this->name, sizeof(this->name), "%s", key);
    if (len < 0 || len >= (int) sizeof(this->name)) {
        memset(this, 0, sizeof(struct Mediator));
        fprintf(stderr, "\033[0;31m[PureMVC::Mediator::init] Error: Mediator name truncated: '%s' (max %zu chars).\033[0m\n", key, sizeof(this->name) - 1u);
        return NULL;
    }

    this->component = component;

    puremvc_notifier_init((struct INotifier *) &this->notifier);

    return (struct IMediator *) this;
}
