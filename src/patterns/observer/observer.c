/**
* @file observer.c
* @internal
* @brief Observer Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/observer.h"

static void *getContext(const struct IObserver *self) {
    const struct Observer *this = (struct Observer *) self;
    return this->context;
}

static void setContext(struct IObserver *self, void *notifyContext) {
    struct Observer *this = (struct Observer *) self;
    this->context = notifyContext;
}

static void (*getNotify(const struct IObserver *self))(const void *context, struct INotification *notification) {
    const struct Observer *this = (struct Observer *) self;
    return this->notify;
}

static void setNotify(struct IObserver *self, void (*notify)(const void *context, struct INotification *notification)) {
    struct Observer *this = (struct Observer *) self;
    this->notify = notify;
}

static void notifyObserver(const struct IObserver *self, struct INotification *notification) {
    const struct Observer *this = (struct Observer *) self;
    if (this->notify == NULL && this->context == NULL) return;
    this->notify(this->context, notification);
}

static bool compareNotifyContext(const struct IObserver *self, const void *context) {
    const struct Observer *this = (struct Observer *) self;
    if (this->context == NULL || context == NULL) return false;
    return this->context == context;
}

struct IObserver *puremvc_observer(struct Observer *const observer, void (*notify)(const void *context, struct INotification *notification), void *context) {
    observer->base.getContext = getContext;
    observer->base.setContext = setContext;
    observer->base.getNotify = getNotify;
    observer->base.setNotify = setNotify;
    observer->base.notifyObserver = notifyObserver;
    observer->base.compareNotifyContext = compareNotifyContext;

    observer->context = context;
    observer->notify = notify;

    return &observer->base;
}

void puremvc_observer_deinit(struct Observer *observer) {
    observer->base = (struct IObserver){0};
    observer->context = NULL;
    observer->notify = NULL;
}
