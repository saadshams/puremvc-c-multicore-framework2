#include "view_test_mediator5.h"
#include "view_test.h"

static const char *const *listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *const interests[] = {NOTE5, NULL};
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    if (notification == NULL) return;
    (void)notification;
    ((struct ViewTest *) self->getComponent(self))->counter++;
}

struct IMediator *view_test_mediator5(void *buffer, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(buffer, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->handleNotification = handleNotification;
    return self;
}
