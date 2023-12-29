#ifndef PUREMVC_FACADE_H
#define PUREMVC_FACADE_H

#include "controller.h"
#include "model.h"
#include "view.h"

struct Facade {

    /** The Multiton Key for this app */
    const char *multitonKey;

    /** References to Model, View and Controller */
    struct Controller *controller;

    struct Model *model;

    struct View *view;

    void (*initializeFacade)(struct Facade *self);

    void (*initializeController)(struct Facade *self);

    void (*initializeModel)(struct Facade *self);

    void (*initializeView)(struct Facade *self);

    void (*registerCommand)(struct Facade *self, const char *notificationName, struct SimpleCommand *(*factory)(void));

    void (*removeCommand)(struct Facade *self, const char *notificationName);

    bool (*hasCommand)(struct Facade *self, const char *notificationName);

    void (*registerProxy)(struct Facade *self, struct Proxy *proxy);

    struct Proxy *(*retrieveProxy)(struct Facade *self, const char *proxyName);

    struct Proxy *(*removeProxy)(struct Facade *self, const char *proxyName);

    bool (*hasProxy)(struct Facade *self, const char *proxyName);

    void (*registerMediator)(struct Facade *self, struct Mediator *mediator);

    struct Mediator *(*retrieveMediator)(struct Facade *self, const char *mediatorName);

    struct Mediator *(*removeMediator)(struct Facade *self, const char *mediatorName);

    bool (*hasMediator)(struct Facade *self, const char *mediatorName);

    void (*sendNotification)(struct Facade *self, const char *notificationName, void *body, char *type);

    void (*notifyObservers)(struct Facade *self, struct Notification *notification);

    void (*initializeNotifier)(struct Facade *self, const char *key);
};

struct Facade *puremvc_facade_init(struct Facade *self);

struct Facade *puremvc_facade_alloc(const char *key);

struct Facade *puremvc_facade_new(const char *key);

void puremvc_facade_free(struct Facade **self);

struct Facade *puremvc_facade_getInstance(const char *key, struct Facade *(*factory)(const char *));

bool puremvc_facade_hasCore(const char *key);

void puremvc_facade_removeFacade(const char *key);

#endif //PUREMVC_FACADE_H
