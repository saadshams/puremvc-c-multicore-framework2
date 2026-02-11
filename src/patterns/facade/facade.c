/**
* @file Facade.c
* @internal
* @brief Facade Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>
#include <string.h>

#include "facade.h"

#include <puremvc/i_mutex.h>
#include "puremvc/i_facade.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_model.h"
#include "puremvc/i_view.h"
#include "puremvc/i_notification.h"

// instanceMap
static struct FacadeMap **instanceMap = NULL;

// mutex for facadeMap
static Mutex facadeMapMutex;
static MutexOnce facadeMutexOnce = MUTEX_ONCE_INIT;

static void initializeFacade(struct IFacade *self, struct FacadeMap **facadeMap) {
    if (!self || !facadeMap) return;

    self->initializeModel(self, (*facadeMap)->modelMap);
    self->initializeView(self, (*facadeMap)->viewMap);
    self->initializeController(self, (*facadeMap)->controllerMap);
}

static void initializeController(struct IFacade *self, struct ControllerMap **controllerMap) {
    struct Facade *this = (struct Facade *) self;
    if (this->controller != NULL) return;

    this->controller = puremvc_controller_getInstance(controllerMap, this->multitonKey);
    this->controller->initializeController(this->controller);
}

static void initializeModel(struct IFacade *self, struct ModelMap **modelMap) {
    struct Facade *this = (struct Facade *) self;
    if (this->model != NULL) return;

    this->model = puremvc_model_getInstance(modelMap, this->multitonKey);
    this->model->initializeModel(this->model);
}

static void initializeView(struct IFacade *self, struct ViewMap **viewMap) {
    struct Facade *this = (struct Facade *) self;
    if (this->view != NULL) return;

    this->view = puremvc_view_getInstance(viewMap, this->multitonKey);
    this->view->initializeView(this->view);
}

static bool registerCommand(const struct IFacade *self, const char *notificationName, struct ICommand *(*factory)(struct ICommand *)) {
    const struct Facade *this = (struct Facade *) self;
    return this->controller->registerCommand(this->controller, notificationName, factory);
}

static bool hasCommand(const struct IFacade *self, const char *notificationName) {
    const struct Facade *this = (struct Facade *) self;
    return this->controller->hasCommand(this->controller, notificationName);
}

static bool removeCommand(const struct IFacade *self, const char *notificationName, struct ICommand *(**factory)(struct ICommand *)) {
    const struct Facade *this = (struct Facade *) self;
    return this->controller->removeCommand(this->controller, notificationName, factory);
}

static bool registerProxy(const struct IFacade *self, struct IProxy *(*factory)(struct IProxy *proxy, const char *name, void *data), const char *name, void *data) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->registerProxy(this->model, factory, name, data);
}

static struct IProxy *retrieveProxy(const struct IFacade *self, const char *proxyName) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->retrieveProxy(this->model, proxyName);
}

static bool hasProxy(const struct IFacade *self, const char *proxyName) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->hasProxy(this->model, proxyName);
}

static bool removeProxy(const struct IFacade *self, const char *proxyName, struct IProxy **out) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->removeProxy(this->model, proxyName, out);
}

static bool registerMediator(const struct IFacade *self, struct IMediator *(*factory)(struct IMediator *mediator, const char *name, void *component), const char *name, void *component) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->registerMediator(this->view, factory, name, component);
}

static struct IMediator *retrieveMediator(const struct IFacade *self, const char *mediatorName) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->retrieveMediator(this->view, mediatorName);
}

static bool hasMediator(const struct IFacade *self, const char *mediatorName) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->hasMediator(this->view, mediatorName);
}

static bool removeMediator(const struct IFacade *self, const char *mediatorName, struct IMediator **out) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->removeMediator(this->view, mediatorName, out);
}

static void notifyObservers(const struct IFacade *self, struct INotification *notification) {
    const struct Facade *this = (struct Facade *) self;
    this->view->notifyObservers(this->view, notification);
}

static void sendNotification(const struct IFacade *self, const char *notificationName, void *body, const char *type) {
    struct INotification *notification = puremvc_notification_init((struct INotification *) &(struct Notification){0}, notificationName, body, type);
    self->notifyObservers(self, notification);
}

static void puremvc_facade_init(struct IFacade *facade, const char *key) {
    struct Facade *this = (struct Facade *) facade;

    memset(this, 0, sizeof(struct Facade));

    facade->initializeFacade = initializeFacade;
    facade->initializeController = initializeController;
    facade->initializeModel = initializeModel;
    facade->initializeView = initializeView;
    facade->registerCommand = registerCommand;
    facade->hasCommand = hasCommand;
    facade->removeCommand = removeCommand;
    facade->registerProxy = registerProxy;
    facade->retrieveProxy = retrieveProxy;
    facade->hasProxy = hasProxy;
    facade->removeProxy = removeProxy;
    facade->registerMediator = registerMediator;
    facade->retrieveMediator = retrieveMediator;
    facade->hasMediator = hasMediator;
    facade->removeMediator = removeMediator;
    facade->notifyObservers = notifyObservers;
    facade->sendNotification = sendNotification;

    this->multitonKey = key;
}

static void dispatchOnce(void) {
    mutex_init(&facadeMapMutex);
}

struct IFacade *puremvc_facade_getInstance(struct FacadeMap **facadeMap, const char *key) {
    if (facadeMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    if (instanceMap == NULL)
        instanceMap = facadeMap;

    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    size_t i = 0;
    for (; instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) { // find facade
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
            mutex_unlock(&facadeMapMutex);
            return instanceMap[i]->facade;
        }
    }

    if (instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: FacadeMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&facadeMapMutex);
        return NULL;
    }

    instanceMap[i]->key = key; // init
    puremvc_facade_init(facadeMap[i]->facade, key);
    instanceMap[i]->facade->initializeFacade(instanceMap[i]->facade, facadeMap);

    mutex_unlock(&facadeMapMutex);
    return instanceMap[i]->facade;
}

bool puremvc_facade_hasCore(struct FacadeMap **facadeMap, const char *key) {
    mutex_lock_shared(&facadeMapMutex);
    bool exists = false;
    for (size_t i = 0; facadeMap != NULL && facadeMap[i]->key != NULL; i++) {
        if (facadeMap[i]->key == key || strcmp(facadeMap[i]->key, key) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&facadeMapMutex);
    return exists;
}

bool puremvc_facade_removeFacade(const char *key, struct IFacade **out) {
    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeFacade] FATAL: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeFacade] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return false;
    }

    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    puremvc_model_removeModel(key, NULL);
    puremvc_view_removeView(key, NULL);
    puremvc_controller_removeController(key, NULL);

    size_t index = 0;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) { // find facade
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
            instanceMap[i]->key = NULL; // clear model
            if (out != NULL)
                *out = instanceMap[i]->facade;
        } else {
            if (index != i) { // shift left
                *instanceMap[index] = *instanceMap[i];
                instanceMap[i]->key = NULL;
            }
            index++;
        }
    }

    if (index == 0) // all keys were removed; reset
        instanceMap = NULL;

    mutex_unlock(&facadeMapMutex);

    return true;
}
