#include <stdio.h>

#include "view_test_mediator6.h"
#include "puremvc/facade.h"

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = { NOTE6, NULL };
    return interests;
}

static void handleNotification(const struct Mediator *self, struct Notification notification) {
    struct ViewTest *component = self->getComponent(self);
    const char *name = self->getName(self);

    for (size_t i = 0; i < MEDIATOR_MAP_SIZE; i++) {
        // Find the first empty slot (assumes first char is '\0')
        if (component->deferred[i][0] == '\0') {
            snprintf(component->deferred[i], NAME_SIZE, "%s", name);
            break;
        }
    }
}

static void onRemove(struct Mediator *self) {
    ((struct ViewTest *) self->getComponent(self))->counter++;
}

struct Mediator view_test_mediator6(const char *name, struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(name, component);
    mediator.listNotificationInterests = listNotificationInterests;
    mediator.handleNotification = handleNotification;
    mediator.onRemove = onRemove;
    return mediator;
}
