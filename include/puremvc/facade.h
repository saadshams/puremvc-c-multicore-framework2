/**
* @file Facade.h
* @internal
* @brief Facade Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"
#include "controller.h"
#include "model.h"
#include "view.h"

struct Facade {
    char multitonKey[KEY_SIZE];

    struct Controller *controller;
    struct Model *model;
    struct View *view;
    
    void (*initializeFacade)(struct Facade *self);
    
    void (*initializeController)(struct Facade *self);
    
    void (*initializeModel)(struct Facade *self);
    
    void (*initializeView)(struct Facade *self);
    
    void (*registerCommand)(const struct Facade *self, const char *notificationName, struct SimpleCommand(*factory)());
    
    bool (*hasCommand)(const struct Facade *self, const char *notificationName);

    void (*removeCommand)(const struct Facade *self, const char *notificationName);
    
    void (*registerProxy)(const struct Facade *self, struct Proxy proxy);

    struct Proxy *(*retrieveProxy)(const struct Facade *self, const char *proxyName);

    bool (*hasProxy)(const struct Facade *self, const char *proxyName);

    struct Proxy(*removeProxy)(const struct Facade *self, const char *proxyName);
    
    void (*registerMediator)(const struct Facade *self, struct Mediator mediator);

    struct Mediator *(*retrieveMediator)(const struct Facade *self, const char *mediatorName);

    bool (*hasMediator)(const struct Facade *self, const char *mediatorName);
    
    struct Mediator(*removeMediator)(const struct Facade *self, const char *mediatorName);
    
    void (*notifyObservers)(const struct Facade *self, const struct Notification notification);
    
    void (*sendNotification)(const struct Facade *self, const char *notificationName, void *body, const char *type);
};

struct Facade puremvc_facade(const char *key);

struct Facade *puremvc_facade_getInstance(const char *key, struct Facade(*factory)(const char *));

void puremvc_facade_removeFacade(const char *key);

bool puremvc_facade_hasCore(const char *key);
