#include "view_test_mediator.h"

static const char *const *listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *const interests[] = {"ABC", "DEF", "GHI", NULL};
    return interests;
}

struct IMediator *view_test_mediator(void *buffer, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(buffer, name, component);
    self->listNotificationInterests = listNotificationInterests;
    return self;
}
