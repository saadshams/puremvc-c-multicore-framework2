#include <stdio.h>

#include "view_test_mediator6.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    static const char *interests[] = { NOTE6, NULL };
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    struct ViewTest *component = self->getComponent(self);
    char *name = (char *) self->getName(self);

    for (size_t i = 0; component->deferred[i] != NULL; i++) {
        if (component->deferred[i][0] != '\0')
            continue;
        snprintf(component->deferred[i], KEY_SIZE, "%s", name);
        break;
    }
}

static void onRemove(struct IMediator *self) {
    ((struct ViewTest *) self->getComponent(self))->counter++;
}

struct IMediator *view_test_mediator6(struct Mediator *const mediator, const char *name, struct ViewTest *component) {
    puremvc_mediator(mediator, name, component);
    mediator->base.listNotificationInterests = listNotificationInterests;
    mediator->base.handleNotification = handleNotification;
    mediator->base.onRemove = onRemove;
    return &mediator->base;
}
