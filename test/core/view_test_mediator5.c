#include "view_test_mediator5.h"

#include <stdlib.h>
#include <string.h>

static const char **listNotificationInterests(struct Mediator *self) {
    const char **interests = malloc(2 * sizeof(char *));
    interests[0] = NOTE5;
    interests[1] = NULL;
    return interests;
}

static void handleNotification(struct Mediator *self, struct Notification *notification) {
    ViewTest *viewTest = self->getViewComponent(self);
    viewTest->counter++;
}

ViewTestMediator5 *test_mediator5_init(ViewTestMediator5 *self) {
    puremvc_mediator_init(&self->mediator);
    self->mediator.listNotificationInterests = listNotificationInterests;
    self->mediator.handleNotification = handleNotification;
    return self;
}

ViewTestMediator5 *test_mediator5_alloc(ViewTest *view) {
    ViewTestMediator5 *self = malloc(sizeof(ViewTestMediator5));
    self->mediator.mediatorName = strdup(ViewTestMediator5_NAME);
    self->mediator.viewComponent = view;
    return self;
}

ViewTestMediator5 *test_mediator5_new(ViewTest *view) {
    return test_mediator5_init(test_mediator5_alloc(view));
}

void test_mediator5_free(ViewTestMediator5 **self) {
    free((void *) (*self)->mediator.mediatorName);
    free(*self);
    *self = NULL;
}