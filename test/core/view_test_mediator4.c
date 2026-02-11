#include "view_test_mediator4.h"
#include "view_test.h"

static const char *const *listNotificationInterests(const struct IMediator *self) {
    static const char *const interests[] = {NOTE4, NULL};
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

struct IMediator *view_test_mediator4(void *buffer, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(buffer, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->onRegister = onRegister;
    self->onRemove = onRemove;
    return self;
}
