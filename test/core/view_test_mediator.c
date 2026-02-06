#include "view_test_mediator.h"

static const char **listNotificationInterests(const struct IMediator *self) {
    (void)self;
    static const char *interests[] = {"ABC", "DEF", "GHI", NULL};
    return interests;
}

struct IMediator *view_test_mediator(struct Mediator *const mediator, struct ViewComponent *component) {
    puremvc_mediator(mediator, view_test_mediator_NAME, component);
    mediator->base.listNotificationInterests = listNotificationInterests;
    return &mediator->base;
}
