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
    struct IMediator *base = puremvc_mediator(mediator, view_test_mediator4_NAME, component);
    base->listNotificationInterests = listNotificationInterests;
    base->onRegister = onRegister;
    base->onRemove = onRemove;
    return base;
}
