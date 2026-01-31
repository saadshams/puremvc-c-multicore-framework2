#include "view_test_mediator2.h"

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = {NOTE1, NOTE2, NULL};
    return interests;
}

static void handleNotification(const struct Mediator *self, const struct Notification notification) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->lastNotification = notification.getName(&notification);
}

struct Mediator view_test_mediator2(struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(view_test_mediator2_NAME, component);
    mediator.listNotificationInterests = listNotificationInterests;
    mediator.handleNotification = handleNotification;
    return mediator;
}
