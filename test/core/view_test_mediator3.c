#include "view_test_mediator3.h"
#include "view_test.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = {NOTE3, NULL};
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->lastNotification = notification->getName(notification);
}

struct IMediator *view_test_mediator3(void *buffer, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(buffer, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->handleNotification = handleNotification;
    return self;
}
