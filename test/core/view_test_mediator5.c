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

struct IMediator *view_test_mediator5(struct Mediator *const mediator, struct ViewTest *component) {
    struct IMediator *base = puremvc_mediator(mediator, view_test_mediator5_NAME, component);
    base->listNotificationInterests = listNotificationInterests;
    base->handleNotification = handleNotification;
    return base;
}
