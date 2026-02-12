/**
* @file Facade.c
* @internal
* @brief Facade Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "facade.h"

#include <puremvc/i_mutex.h>
#include "puremvc/i_facade.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_model.h"
#include "puremvc/i_view.h"
#include "puremvc/i_notification.h"

#include <alloca.h>
#include <stdio.h>
#include <string.h>

// instanceMap
static struct FacadeMap **instanceMap = NULL;

// mutex for facadeMap
static Mutex facadeMapMutex;
static MutexOnce facadeMutexOnce = MUTEX_ONCE_INIT;

static void initializeFacade(struct IFacade *self, struct IModel *model, struct IView *view, struct IController *controller) {
    if (self == NULL) return;
    self->initializeModel(self, model);
    self->initializeView(self, view);
    self->initializeController(self, controller);
}

static void initializeController(struct IFacade *self, struct IController *controller) {
    struct Facade *this = (struct Facade *) self;
    if (this->controller != NULL || controller == NULL) return;
    this->controller = controller;
}

static void initializeModel(struct IFacade *self, struct IModel *model) {
    struct Facade *this = (struct Facade *) self;
    if (this->model != NULL || model == NULL) return;
    this->model = model;
}

static void initializeView(struct IFacade *self, struct IView *view) {
    struct Facade *this = (struct Facade *) self;
    if (this->view != NULL || view == NULL) return;
    this->view = view;
}

static bool registerCommand(const struct IFacade *self, const char *notificationName, struct ICommand *(*factory)(void *buffer)) {
    const struct Facade *this = (struct Facade *) self;
    return this->controller->registerCommand(this->controller, notificationName, factory);
}

static bool hasCommand(const struct IFacade *self, const char *notificationName) {
    const struct Facade *this = (struct Facade *) self;
    return this->controller->hasCommand(this->controller, notificationName);
}

static bool removeCommand(const struct IFacade *self, const char *notificationName, struct ICommand *(**out)(void *)) {
    const struct Facade *this = (struct Facade *) self;
    return this->controller->removeCommand(this->controller, notificationName, out);
}

static bool registerProxy(const struct IFacade *self, struct IProxy *(*factory)(void *buffer, const char *name, void *data), const char *name, void *data) {
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

static bool registerMediator(const struct IFacade *self, struct IMediator *(*factory)(void *buffer, const char *name, void *component), const char *name, void *component) {
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
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), notificationName, body, type);
    self->notifyObservers(self, notification);
}

size_t puremvc_facade_size() {
    return (sizeof(struct Facade) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct IFacade *puremvc_facade_init(void *buffer, const char *key) {
    struct Facade *this = (struct Facade *) buffer;

    memset(this, 0, sizeof(struct Facade));

    this->base.initializeFacade = initializeFacade;
    this->base.initializeController = initializeController;
    this->base.initializeModel = initializeModel;
    this->base.initializeView = initializeView;
    this->base.registerCommand = registerCommand;
    this->base.hasCommand = hasCommand;
    this->base.removeCommand = removeCommand;
    this->base.registerProxy = registerProxy;
    this->base.retrieveProxy = retrieveProxy;
    this->base.hasProxy = hasProxy;
    this->base.removeProxy = removeProxy;
    this->base.registerMediator = registerMediator;
    this->base.retrieveMediator = retrieveMediator;
    this->base.hasMediator = hasMediator;
    this->base.removeMediator = removeMediator;
    this->base.notifyObservers = notifyObservers;
    this->base.sendNotification = sendNotification;

    this->multitonKey = key;

    return (struct IFacade *) this;
}

static void dispatchOnce(void) {
    mutex_init(&facadeMapMutex);
}

struct IFacade *puremvc_facade_getInstance(struct FacadeMap **facadeMap, const char *key) {
    if (facadeMap == NULL && instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    if (facadeMap != NULL) instanceMap = facadeMap; // notifier exception

    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    size_t i = 0;
    for (; instanceMap != NULL && instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) { // find facade
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
            mutex_unlock(&facadeMapMutex);
            return instanceMap[i]->facade;
        }
    }

    if (instanceMap == NULL || instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: FacadeMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&facadeMapMutex);
        return NULL;
    }

    if (instanceMap[i]->facade == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: Missing Facade storage; skipping registration.\033[0m\n");
        return NULL;
    }

    instanceMap[i]->key = key; // init
    puremvc_facade_init(instanceMap[i]->facade, key);

    mutex_unlock(&facadeMapMutex);
    return instanceMap[i]->facade;
}

bool puremvc_facade_hasCore(const char *key) {
    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::hasFacade] FATAL: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock_shared(&facadeMapMutex);
    bool exists = false;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) {
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
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
            instanceMap[i]->key = NULL; // remove
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

void puremvc_facade_reset() {
    mutex_lock(&facadeMapMutex);
    instanceMap = NULL;
    mutex_unlock(&facadeMapMutex);
}
