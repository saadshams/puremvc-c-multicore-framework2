/**
* @file observer.c
* @internal
* @brief Observer Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "observer.h"

#include <string.h>

static void *getContext(const struct IObserver *self) {
    const struct Observer *this = (const struct Observer *) self;
    return this->context;
}

static void setContext(struct IObserver *self, void *notifyContext) {
    struct Observer *this = (struct Observer *) self;
    this->context = notifyContext;
}

static bool (*getNotify(const struct IObserver *self))(const void *context, const struct INotification *notification) {
    const struct Observer *this = (const struct Observer *) self;
    return this->notify;
}

static void setNotify(struct IObserver *self, bool (*notify)(const void *context, const struct INotification *notification)) {
    struct Observer *this = (struct Observer *) self;
    this->notify = notify;
}

static void notifyObserver(const struct IObserver *self, const struct INotification *notification) {
    const struct Observer *this = (const struct Observer *) self;
    if (this->notify == NULL && this->context == NULL) return;
    this->notify(this->context, notification);
}

static bool compareNotifyContext(const struct IObserver *self, const void *context) {
    const struct Observer *this = (const struct Observer *) self;
    if (this->context == NULL || context == NULL) return false;
    return this->context == context;
}

size_t puremvc_observer_size() {
    return (sizeof(struct Observer) + (sizeof(void *) - 1u)) & ~(sizeof(void *) - 1u);
}

struct IObserver *puremvc_observer_init(void *buffer, bool (*notify)(const void *context, const struct INotification *notification), void *context) {
    struct Observer *this = (struct Observer *) buffer;

    memset(this, 0, sizeof(struct Observer));

    this->base.getContext = getContext;
    this->base.setContext = setContext;
    this->base.getNotify = getNotify;
    this->base.setNotify = setNotify;
    this->base.notifyObserver = notifyObserver;
    this->base.compareNotifyContext = compareNotifyContext;

    this->context = context;
    this->notify = notify;

    return (struct IObserver *) this;
}
