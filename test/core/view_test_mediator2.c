#include "view_test_mediator2.h"
#include "view_test.h"

static const char *const *listNotificationInterests(const struct IMediator *self) {
    static const char *const interests[] = {NOTE1, NOTE2, NULL};
    return interests;
}

static bool handleNotification(const struct IMediator *self, struct INotification *notification) {
    if (notification == NULL) return false;
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->lastNotification = notification->getName(notification);
    return true;
}

struct IMediator *view_test_mediator2(void *buffer, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(buffer, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->handleNotification = handleNotification;
    return self;
}
