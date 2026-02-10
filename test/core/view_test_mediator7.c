#include <stdio.h>

#include "view_test_mediator7.h"
#include "view_test.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = { NOTE7, NULL };
    return interests;
}

struct IMediator *view_test_mediator7(void *buffer, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(buffer, name, component);
    self->listNotificationInterests = listNotificationInterests;
    return self;
}
