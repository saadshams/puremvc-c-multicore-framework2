#include <stdio.h>

#include "view_test_mediator7.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = { NOTE7, NULL };
    return interests;
}

struct Mediator view_test_mediator7(const char *name, struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(name, component);
    mediator.base.listNotificationInterests = listNotificationInterests;
    return mediator;
}
