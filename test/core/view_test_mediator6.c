#include "view_test_mediator6.h"
#include "facade.h"
#include "notifier.h"
#include <stdlib.h>
#include <string.h>

static const char **listNotificationInterests(struct Mediator *self) {
    const char **interests = malloc(2 * sizeof(char *));
    interests[0] = NOTE6;
    interests[1] = NULL;
    return interests;
}

static void handleNotification(struct Mediator *self, struct Notification *notification) {
    struct Facade *facade = self->notifier->getFacade(self->notifier);
    struct Mediator *mediator = facade->removeMediator(facade, self->mediatorName);
    test_mediator6_free((ViewTestMediator6 **) &mediator);
}

static void onRemove(struct Mediator *self) {
    ((ViewTest *) self->viewComponent)->counter++;
}

ViewTestMediator6 *test_mediator6_init(ViewTestMediator6 *self) {
    puremvc_mediator_init(&self->mediator);
    self->mediator.listNotificationInterests = listNotificationInterests;
    self->mediator.handleNotification = handleNotification;
    self->mediator.onRemove = onRemove;
    return self;
}

ViewTestMediator6 *test_mediator6_alloc(const char *mediatorName, ViewTest *view) {
    ViewTestMediator6 *self = malloc(sizeof(ViewTestMediator6));
    self->mediator.mediatorName = strdup(mediatorName);
    self->mediator.viewComponent = view;
    return self;
}

ViewTestMediator6 *test_mediator6_new(const char *mediatorName, ViewTest *view) {
    return test_mediator6_init(test_mediator6_alloc(mediatorName, view));
}

void test_mediator6_free(ViewTestMediator6 **self) {
    free((void *) (*self)->mediator.mediatorName);
    free(*self);
    *self = NULL;
}
