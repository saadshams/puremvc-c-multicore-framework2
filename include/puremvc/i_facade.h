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

#include "i_controller.h"
#include "i_model.h"
#include "i_view.h"
#include "i_command.h"
#include "i_mediator.h"
#include "i_notification.h"
#include "i_proxy.h"

/**
 * @struct IFacade
 * @brief Represents the Facade in the PureMVC framework.
 *
 * The Facade provides a unified interface to the Model, View,
 * and Controller layers. It manages commands, proxies, mediators,
 * and notification dispatching for a given multiton key.
 */

struct FacadeMap {
    const char *key;
    struct IFacade *facade;

    struct ModelMap **modelMap;
    struct ViewMap **viewMap;
    struct ControllerMap **controllerMap;
};

struct IFacade {
    void (*initializeFacade)(struct IFacade *self, struct IModel *model, struct IView *view, struct IController *controller);
    
    void (*initializeController)(struct IFacade *self, struct IController *controller);

    void (*initializeModel)(struct IFacade *self, struct IModel *model);
    
    void (*initializeView)(struct IFacade *self, struct IView *view);

    bool (*registerCommand)(const struct IFacade *self, const char *notificationName, struct ICommand *(*factory)(void *buffer));
    
    bool (*hasCommand)(const struct IFacade *self, const char *notificationName);

    bool (*removeCommand)(const struct IFacade *self, const char *notificationName, struct ICommand *(**out)(void *buffer));

    bool (*registerProxy)(const struct IFacade *self, struct IProxy *(*factory)(void *buffer, const char *name, void *data), const char *name, void *data);

    struct IProxy *(*retrieveProxy)(const struct IFacade *self, const char *proxyName);

    bool (*hasProxy)(const struct IFacade *self, const char *proxyName);

    bool (*removeProxy)(const struct IFacade *self, const char *proxyName, struct IProxy **proxy);

    bool (*registerMediator)(const struct IFacade *self, struct IMediator *(*factory)(void *buffer, const char *name, void *component), const char *name, void *component);

    struct IMediator *(*retrieveMediator)(const struct IFacade *self, const char *mediatorName);

    bool (*hasMediator)(const struct IFacade *self, const char *mediatorName);
    
    bool (*removeMediator)(const struct IFacade *self, const char *mediatorName, struct IMediator **mediator);
    
    void (*notifyObservers)(const struct IFacade *self, struct INotification *notification);

    void (*sendNotification)(const struct IFacade *self, const char *notificationName, void *body, const char *type);
};

size_t puremvc_facade_size();

struct IFacade *puremvc_facade_init(void *buffer, const char *key);

struct IFacade *puremvc_facade_getInstance(struct FacadeMap **facadeMap, const char *key);

bool puremvc_facade_hasCore(const char *key);

bool puremvc_facade_removeFacade(const char *key, struct IFacade **out);
