#include "view_test_mediator3.h"

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = {NOTE3, NULL};
    return interests;
}

static void handleNotification(const struct Mediator *self, struct Notification notification) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->lastNotification = notification.getName(&notification);
}

struct Mediator view_test_mediator3(struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(view_test_mediator3_NAME, component);
    mediator.listNotificationInterests = listNotificationInterests;
    mediator.handleNotification = handleNotification;
    return mediator;
}
