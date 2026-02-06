#include "view_test_mediator3.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = {NOTE3, NULL};
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->lastNotification = notification->getName(notification);
}

struct IMediator *view_test_mediator3(struct Mediator *const mediator, struct ViewTest *component) {
    puremvc_mediator(mediator, view_test_mediator3_NAME, component);
    mediator->base.listNotificationInterests = listNotificationInterests;
    mediator->base.handleNotification = handleNotification;
    return &mediator->base;
}
