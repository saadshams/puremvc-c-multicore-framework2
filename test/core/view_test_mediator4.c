#include "view_test_mediator4.h"
#include "view_test.h"

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

struct IMediator *view_test_mediator4(struct IMediator *const mediator, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(mediator, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->onRegister = onRegister;
    self->onRemove = onRemove;
    return self;
}
