#include <stdlib.h>

#include "view_test_mediator4.h"

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = {NOTE4, NULL};
    return interests;
}

static void onRegister(struct Mediator *self) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->onRegisterCalled = true;
}

static void onRemove(struct Mediator *self) {
    struct ViewTest *viewTest = self->getComponent(self);
    viewTest->onRemoveCalled = true;
}

struct Mediator view_test_mediator4(struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(view_test_mediator4_NAME, component);
    mediator.listNotificationInterests = listNotificationInterests;
    mediator.onRegister = onRegister;
    mediator.onRemove = onRemove;
    return mediator;
}
