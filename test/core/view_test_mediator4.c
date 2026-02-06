#include "view_test_mediator4.h"

static const char **listNotificationInterests(const struct IMediator *self) {
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

struct IMediator *view_test_mediator4(struct Mediator *const mediator, struct ViewTest *component) {
    puremvc_mediator(mediator, view_test_mediator4_NAME, component);
    mediator->base.listNotificationInterests = listNotificationInterests;
    mediator->base.onRegister = onRegister;
    mediator->base.onRemove = onRemove;
    return &mediator->base;
}
