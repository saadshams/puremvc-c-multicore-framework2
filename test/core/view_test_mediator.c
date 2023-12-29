#include "view_test_mediator.h"

#include <stdlib.h>
#include <string.h>

static const char **listNotificationInterests(struct Mediator *self) {
    const char **interests = malloc(4 * sizeof(char *));
    interests[0] = "ABC";
    interests[1] = "DEF";
    interests[2] = "GHI";
    interests[3] = NULL;
    return interests;
}

ViewTestMediator *test_mediator_init(ViewTestMediator *self) {
    puremvc_mediator_init(&self->mediator);
    self->mediator.listNotificationInterests = listNotificationInterests;
    return self;
}

ViewTestMediator *test_mediator_alloc(ViewComponent *view) {
    ViewTestMediator *self = malloc(sizeof(ViewTestMediator));
    puremvc_mediator_init(&self->mediator);
    self->mediator.mediatorName = strdup(ViewTestMediator_NAME);
    self->mediator.viewComponent = view;
    return self;
}

ViewTestMediator *test_mediator_new(ViewComponent *viewComponent) {
    return test_mediator_init(test_mediator_alloc(viewComponent));
}

void test_mediator_free(ViewTestMediator **self) {
    free((void *) (*self)->mediator.mediatorName);
    free(*self);
    *self = NULL;
}