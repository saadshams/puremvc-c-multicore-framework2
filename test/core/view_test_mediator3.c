#include "view_test_mediator3.h"

#include <stdlib.h>
#include <string.h>

static const char **listNotificationInterests(struct Mediator *self) {
    // be sure that the mediator has some Observers created
    // in order to test removeMediator
    const char **interests = malloc(2 * sizeof(char *));
    interests[0] = NOTE3;
    interests[1] = NULL;
    return interests;
}

static void handleNotification(struct Mediator *self, struct Notification *notification) {
    ViewTest *viewTest = self->getViewComponent(self);
    viewTest->lastNotification = notification->name;
}

ViewTestMediator3 *test_mediator3_init(ViewTestMediator3 *self) {
    puremvc_mediator_init(&self->mediator);
    self->mediator.listNotificationInterests = listNotificationInterests;
    self->mediator.handleNotification = handleNotification;
    return self;
}

ViewTestMediator3 *test_mediator3_alloc(ViewTest *view) {
    ViewTestMediator3 *self = malloc(sizeof(ViewTestMediator3));
    self->mediator.mediatorName = strdup(ViewTestMediator3_NAME);
    self->mediator.viewComponent = view;
    return self;
}

ViewTestMediator3 *test_mediator3_new(ViewTest *view) {
    return test_mediator3_init(test_mediator3_alloc(view));
}

void test_mediator3_free(ViewTestMediator3 **self) {
    free((void *) (*self)->mediator.mediatorName);
    free(*self);
    *self = NULL;
}