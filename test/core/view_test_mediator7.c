#include <stdio.h>

#include "view_test_mediator7.h"
#include "view_test.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = { NOTE7, NULL };
    return interests;
}

struct IMediator *view_test_mediator7(struct IMediator *const mediator, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(mediator, name, component);
    self->listNotificationInterests = listNotificationInterests;
    return self;
}
