#include "view_test_mediator4.h"

#include <stdlib.h>
#include <string.h>

static const char **listNotificationInterests(struct Mediator *self) {
    // be sure that the mediator has some Observers created
    // in order to test removeMediator
    const char **interests = malloc(2 * sizeof(char *));
    interests[0] = NOTE4;
    interests[1] = NULL;
    return interests;
}

static void handleNotification(struct Mediator *self, struct Notification *notification) {
}

static void onRegister(struct Mediator *self) {
    ViewTest *viewTest = (ViewTest *) self->getViewComponent(self);
    viewTest->onRegisterCalled = true;
}

static void onRemove(struct Mediator *self) {
    ViewTest *viewTest = (ViewTest *) self->getViewComponent(self);
    viewTest->onRemoveCalled = true;
}

ViewTestMediator4 *test_mediator4_init(ViewTestMediator4 *self) {
    puremvc_mediator_init(&self->mediator);
    self->mediator.onRegister = onRegister;
    self->mediator.onRemove = onRemove;
    self->mediator.listNotificationInterests = listNotificationInterests;
    self->mediator.handleNotification = handleNotification;
    return self;
}

ViewTestMediator4 *test_mediator4_alloc(ViewTest *view) {
    ViewTestMediator4 *self = malloc(sizeof(ViewTestMediator4));
    self->mediator.mediatorName = strdup(ViewTestMediator4_NAME);
    self->mediator.viewComponent = view;
    return self;
}

ViewTestMediator4 *test_mediator4_new(ViewTest *view) {
    return test_mediator4_init(test_mediator4_alloc(view));
}

void test_mediator4_free(ViewTestMediator4 **self) {
    free((void *) (*self)->mediator.mediatorName);
    free(*self);
    *self = NULL;
}