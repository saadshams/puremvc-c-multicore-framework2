#include <stdio.h>

#include "view_test_mediator7.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = { NOTE7, NULL };
    return interests;
}

struct IMediator *view_test_mediator7(struct Mediator *const mediator, const char *name, struct ViewTest *component) {
    struct IMediator *base = puremvc_mediator(mediator, name, component);
    base->listNotificationInterests = listNotificationInterests;
    return base;
}
