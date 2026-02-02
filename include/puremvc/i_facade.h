/**
 * @file i_facade.h
 * @ingroup PureMVC
 * @brief IFacade Interface
 *
 * @author Saad Shams <saad.shams@puremvc.org>
 * @copyright BSD 3-Clause License
 */
#pragma once

#include <stdbool.h>

#include "simple_command.h"
#include "i_mediator.h"
#include "mediator.h"
#include "i_proxy.h"
#include "proxy.h"
#include "i_notification.h"

/**
 * @struct IFacade
 * @brief Represents the Facade in the PureMVC framework.
 *
 * The Facade provides a unified interface to the Model, View,
 * and Controller layers. It manages commands, proxies, mediators,
 * and notification dispatching for a given multiton key.
 */
struct IFacade {

    void (*initializeFacade)(struct IFacade *self);
    
    void (*initializeController)(struct IFacade *self);

    void (*initializeModel)(struct IFacade *self);
    
    void (*initializeView)(struct IFacade *self);

    void (*registerCommand)(const struct IFacade *self, const char *notificationName, struct SimpleCommand(*factory)());
    
    bool (*hasCommand)(const struct IFacade *self, const char *notificationName);

    void (*removeCommand)(const struct IFacade *self, const char *notificationName);

    void (*registerProxy)(const struct IFacade *self, struct Proxy proxy);

    struct IProxy *(*retrieveProxy)(const struct IFacade *self, const char *proxyName);

    bool (*hasProxy)(const struct IFacade *self, const char *proxyName);

    struct Proxy(*removeProxy)(const struct IFacade *self, const char *proxyName);

    void (*registerMediator)(const struct IFacade *self, struct Mediator mediator);

    struct IMediator *(*retrieveMediator)(const struct IFacade *self, const char *mediatorName);

    bool (*hasMediator)(const struct IFacade *self, const char *mediatorName);
    
    struct Mediator(*removeMediator)(const struct IFacade *self, const char *mediatorName);
    
    void (*notifyObservers)(const struct IFacade *self, struct INotification *notification);

    void (*sendNotification)(const struct IFacade *self, const char *notificationName, void *body, const char *type);
};
