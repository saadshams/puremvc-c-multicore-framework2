#include "view_test_mediator6.h"
#include "view_test.h"

static const char *const *listNotificationInterests(const struct IMediator *self) {
    static const char *const interests[] = { NOTE6, NULL };
    return interests;
}

static void handleNotification(const struct IMediator *self, struct INotification *notification) {
    struct ViewTest *component = self->getComponent(self);
    char *name = (char *) self->getName(self);

    for (size_t i = 0; component->deferred[i] != NULL; i++) {
        if (component->deferred[i][0] == '\0') { // Find the first EMPTY slot
            component->deferred[i] = name;
            break;
        }
    }
}

static void onRemove(struct IMediator *self) {
    ((struct ViewTest *) self->getComponent(self))->counter++;
}

struct IMediator *view_test_mediator6(void *buffer, const char *name, void *component) {
    struct IMediator *self = puremvc_mediator_init(buffer, name, component);
    self->listNotificationInterests = listNotificationInterests;
    self->handleNotification = handleNotification;
    self->onRemove = onRemove;
    return self;
}
