#include "view_test_mediator2.h"

#include <stdlib.h>
#include <string.h>

static const char **listNotificationInterests(struct Mediator *self) {
    // be sure that the mediator has some Observers created
    // in order to test removeMediator
    const char **interests = malloc(3 * sizeof(char *));
    interests[0] = NOTE1;
    interests[1] = NOTE2;
    interests[2] = NULL;
    return interests;
}

static void handleNotification(struct Mediator *self, struct Notification *notification) {
    ViewTest *viewTest = self->getViewComponent(self);
    viewTest->lastNotification = notification->name;
}

ViewTestMediator2 *test_mediator2_init(ViewTestMediator2 *self) {
    puremvc_mediator_init(&self->mediator);
    self->mediator.listNotificationInterests = listNotificationInterests;
    self->mediator.handleNotification = handleNotification;
    return self;
}

ViewTestMediator2 *test_mediator2_alloc(ViewTest *view) {
    ViewTestMediator2 *self = malloc(sizeof(ViewTestMediator2));
    self->mediator.mediatorName = strdup(ViewTestMediator2_NAME);
    self->mediator.viewComponent = view;
    return self;
}

ViewTestMediator2 *test_mediator2_new(ViewTest *view) {
    return test_mediator2_init(test_mediator2_alloc(view));
}

void test_mediator2_free(ViewTestMediator2 **self) {
    free((void *) (*self)->mediator.mediatorName);
    free(*self);
    *self = NULL;
}