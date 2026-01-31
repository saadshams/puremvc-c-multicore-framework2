#include <stdio.h>

#include "view_test_mediator7.h"

static const char **listNotificationInterests(const struct Mediator *self) {
    static const char *interests[] = { NOTE7, NULL };
    return interests;
}

static void handleNotification(const struct Mediator *self, struct Notification notification) {

}

static void onRemove(struct Mediator *self) {

}

struct Mediator view_test_mediator7(const char *name, struct ViewTest *component) {
    struct Mediator mediator = puremvc_mediator(name, component);
    mediator.listNotificationInterests = listNotificationInterests;
    mediator.handleNotification = handleNotification;
    mediator.onRemove = onRemove;
    return mediator;
}
