#include "view_test_mediator5.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = {NOTE5, NULL};
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    (void)notification;
    ((struct ViewTest *) self->getComponent(self))->counter++;
}

struct Mediator view_test_mediator5(struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(view_test_mediator5_NAME, component);
    mediator.base.listNotificationInterests = listNotificationInterests;
    mediator.base.handleNotification = handleNotification;
    return mediator;
}
