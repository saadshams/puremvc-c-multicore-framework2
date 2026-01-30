/**
* @file Facade.c
* @internal
* @brief Facade Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <puremvc/mutex.h>
#include "puremvc/facade.h"
#include "puremvc/controller.h"
#include "puremvc/model.h"
#include "puremvc/view.h"

// The Multiton Facade instanceMap.
static struct Facade instanceMap[INSTANCE_MAP_SIZE];

// mutex for instanceMap
static Mutex mutex;
static MutexOnce token = MUTEX_ONCE_INIT;

static void initializeFacade(struct Facade *self) {
    self->initializeModel(self);
    self->initializeController(self);
    self->initializeView(self);
}

static void initializeController(struct Facade *self) {
    if (self->controller != NULL) return;
    self->controller = puremvc_controller_getInstance(self->multitonKey, puremvc_controller);
    self->controller->initializeController(self->controller);
}

static void initializeModel(struct Facade *self) {
    if (self->model != NULL) return;
    self->model = puremvc_model_getInstance(self->multitonKey, puremvc_model);
    self->model->initializeModel(self->model);
}

static void initializeView(struct Facade *self) {
    if (self->view != NULL) return;
    self->view = puremvc_view_getInstance(self->multitonKey, puremvc_view);
    self->view->initializeView(self->view);
}

static void registerCommand(const struct Facade *self, const char *notificationName, struct SimpleCommand(*factory)()) {
    self->controller->registerCommand(self->controller, notificationName, factory);
}

static void removeCommand(const struct Facade *self, const char *notificationName) {
    self->controller->removeCommand(self->controller, notificationName);
}

static bool hasCommand(const struct Facade *self, const char *notificationName) {
    return self->controller->hasCommand(self->controller, notificationName);
}

static void registerProxy(const struct Facade *self, struct Proxy proxy) {
    self->model->registerProxy(self->model, proxy);
}

static struct Proxy *retrieveProxy(const struct Facade *self, const char *proxyName) {
    return self->model->retrieveProxy(self->model, proxyName);
}

static struct Proxy removeProxy(const struct Facade *self, const char *proxyName) {
    return self->model->removeProxy(self->model, proxyName);
}

static bool hasProxy(const struct Facade *self, const char *proxyName) {
    return self->model->hasProxy(self->model, proxyName);
}

static void registerMediator(const struct Facade *self, const struct Mediator mediator) {
    self->view->registerMediator(self->view, mediator);
}

static struct Mediator *retrieveMediator(const struct Facade *self, const char *mediatorName) {
    return self->view->retrieveMediator(self->view, mediatorName);
}

static struct Mediator removeMediator(const struct Facade *self, const char *mediatorName) {
    return self->view->removeMediator(self->view, mediatorName);
}

static bool hasMediator(const struct Facade *self, const char *mediatorName) {
    return self->view->hasMediator(self->view, mediatorName);
}

static void notifyObservers(const struct Facade *self, const struct Notification notification) {
    self->view->notifyObservers(self->view, notification);
}

static void sendNotification(const struct Facade *self, const char *notificationName, void *body, const char *type) {
    const struct Notification notification = puremvc_notification(notificationName, body, type);
    self->notifyObservers(self, notification);
}

struct Facade puremvc_facade(const char *key) {
    struct Facade facade = {0};

    snprintf(facade.multitonKey, KEY_SIZE, "%s", key);

    facade.initializeFacade = initializeFacade;
    facade.initializeController = initializeController;
    facade.initializeModel = initializeModel;
    facade.initializeView = initializeView;
    facade.registerCommand = registerCommand;
    facade.removeCommand = removeCommand;
    facade.hasCommand = hasCommand;
    facade.registerProxy = registerProxy;
    facade.retrieveProxy = retrieveProxy;
    facade.removeProxy = removeProxy;
    facade.hasProxy = hasProxy;
    facade.registerMediator = registerMediator;
    facade.retrieveMediator = retrieveMediator;
    facade.removeMediator = removeMediator;
    facade.hasMediator = hasMediator;
    facade.notifyObservers = notifyObservers;
    facade.sendNotification = sendNotification;
    return facade;
}

static void dispatchOnce() {
    // mutex_init(&mutex);
}

struct Facade *puremvc_facade_getInstance(const char *key, struct Facade(*factory)(const char *)) {
    if (key == NULL || factory == NULL) return NULL;

    size_t i = 0;
    for (; instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            return &instanceMap[i];
        }
    }

    if (i >= INSTANCE_MAP_SIZE) return NULL;

    instanceMap[i] = factory(key);

    // mutex_unlock(&mutex);
    return &instanceMap[i];
}

bool puremvc_facade_hasCore(const char *key) {
    if (key == NULL) return false;
    mutex_lock_shared(&mutex);
    bool exists = false;
    for (size_t i = 0; i < instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&mutex);
    return exists;
}

void puremvc_facade_removeFacade(const char *key) {
    if (key == NULL) return;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    puremvc_model_removeModel(key);
    puremvc_view_removeView(key);
    puremvc_controller_removeController(key);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) != 0) {
            if (index != i)
                memmove(&instanceMap[index], &instanceMap[i], sizeof(struct Facade));
            index++;
        }
    }
    memset(&instanceMap[index], 0, sizeof(struct Facade));

    // mutex_unlock(&mutex);
}
