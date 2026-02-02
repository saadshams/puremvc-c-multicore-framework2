#include "view_test_mediator4.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = {NOTE4, NULL};
    return interests;
}

static void onRegister(struct IMediator *self) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->onRegisterCalled = true;
}

static void onRemove(struct IMediator *self) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->onRemoveCalled = true;
}

struct Mediator view_test_mediator4(struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(view_test_mediator4_NAME, component);
    mediator.base.listNotificationInterests = listNotificationInterests;
    mediator.base.onRegister = onRegister;
    mediator.base.onRemove = onRemove;
    return mediator;
}
