#include "view_test_mediator2.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    static const char *interests[] = {NOTE1, NOTE2, NULL};
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->lastNotification = notification->getName(notification);
}

struct IMediator *view_test_mediator2(struct Mediator *const mediator, struct ViewTest *component) {
    struct IMediator *base = puremvc_mediator(mediator, view_test_mediator2_NAME, component);
    base->listNotificationInterests = listNotificationInterests;
    base->handleNotification = handleNotification;
    return base;
}
