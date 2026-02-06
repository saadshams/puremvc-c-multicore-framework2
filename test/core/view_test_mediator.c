#include "view_test_mediator.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = {"ABC", "DEF", "GHI", NULL};
    return interests;
}

struct IMediator *view_test_mediator(struct IMediator *const mediator, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(mediator, name, component);
    self->listNotificationInterests = listNotificationInterests;
    return self;
}
