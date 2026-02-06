#include "view_test_mediator5.h"
#include "view_test.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = {NOTE5, NULL};
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    (void)notification;
    ((struct ViewTest *) self->getComponent(self))->counter++;
}

struct IMediator *view_test_mediator5(struct IMediator *const mediator, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(mediator, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->handleNotification = handleNotification;
    return self;
}
