#include "view_test_mediator6.h"
#include "view_test.h"
#include "puremvc/i_facade.h"

static const char *const *listNotificationInterests(const struct IMediator *self) {
    static const char *const interests[] = { NOTE6, NULL };
    return interests;
}

static bool handleNotification(const struct IMediator *self, struct INotification *notification) {
    if (notification == NULL) return false;
    // struct ViewTest *component = self->getComponent(self);
    char *name = (char *) self->getName(self);

    const struct INotifier *notifier = self->getNotifier(self);
    const struct IFacade *facade = notifier->getFacade(notifier);
    facade->removeMediator(facade, name, NULL);

    // for (size_t i = 0; component->deferred[i] != NULL; i++) {
    //     if (component->deferred[i][0] == '\0') { // Find the first empty slot
    //         component->deferred[i] = name;
    //         break;
    //     }
    // }
    return true;
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
