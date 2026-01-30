#include <stdio.h>

#include "view_test_mediator6.h"

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = { NOTE6, NULL };
    return interests;
}

static void handleNotification(const struct Mediator *self, struct Notification notification) {
    // const struct IFacade *facade = self->notifier->getFacade(self->notifier);
    // struct Mediator *mediator = facade->removeMediator(facade, self->getName(self));
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
