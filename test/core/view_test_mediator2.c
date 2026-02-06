#include "view_test_mediator2.h"
#include "view_test.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    static const char *interests[] = {NOTE1, NOTE2, NULL};
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->lastNotification = notification->getName(notification);
}

struct IMediator *view_test_mediator2(struct IMediator *const mediator, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(mediator, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->handleNotification = handleNotification;
    return self;
}
