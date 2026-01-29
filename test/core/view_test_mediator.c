#include <stdlib.h>

#include "view_test_mediator.h"

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = {"ABC", "DEF", "GHI", NULL};
    return interests;
}

struct Mediator view_test_mediator(struct ViewComponent *component) {
    struct Mediator mediator = puremvc_mediator(view_test_mediator_NAME, component);
    mediator.listNotificationInterests = listNotificationInterests;
    return mediator;
}
