/**
* @file Facade.c
* @internal
* @brief Facade Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "facade.h"

#include <puremvc/platform.h>
#include "puremvc/i_facade.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_model.h"
#include "puremvc/i_view.h"
#include "puremvc/i_notification.h"

#include <stdio.h>
#include <string.h>

// instanceMap
static struct FacadeMap **instanceMap = NULL;

// mutex for facadeMap
static Mutex instanceMapMutex;
static MutexOnce mutexOnce = MUTEX_ONCE_INIT;

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
    if (this->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::registerCommand] Error: Controller unavailable to register Command '%s' — skipping registration.\033[0m\n", notificationName);
        return false;
    }
    return this->controller->registerCommand(this->controller, notificationName, factory);
}

static bool hasCommand(const struct IFacade *self, const char *notificationName) {
    const struct Facade *this = (struct Facade *) self;
    if (this->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::hasCommand] Error: Controller unavailable to check registration for Command '%s' — skipping registration check.\033[0m\n", notificationName);
        return false;
    }
    return this->controller->hasCommand(this->controller, notificationName);
}

static bool removeCommand(const struct IFacade *self, const char *notificationName, struct ICommand *(**out)(void *)) {
    const struct Facade *this = (struct Facade *) self;
    if (this->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeCommand] Error: Controller unavailable to remove Command '%s' — skipping removal.\033[0m\n", notificationName);
        return false;
    }
    return this->controller->removeCommand(this->controller, notificationName, out);
}

static bool registerProxy(const struct IFacade *self, struct IProxy *(*factory)(void *buffer, const char *proxyName, void *data), const char *proxyName, void *data) {
    const struct Facade *this = (struct Facade *) self;
    if (this->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::registerProxy] Error: Model unavailable to register Proxy '%s' — skipping registration.\033[0m\n", proxyName);
        return false;
    }
    return this->model->registerProxy(this->model, factory, proxyName, data);
}

static struct IProxy *retrieveProxy(const struct IFacade *self, const char *proxyName) {
    const struct Facade *this = (struct Facade *) self;
    if (this->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::retrieveProxy] Error: Model unavailable to retrieve Proxy '%s' — skipping retrieval.\033[0m\n", proxyName);
        return false;
    }
    return this->model->retrieveProxy(this->model, proxyName);
}

static bool hasProxy(const struct IFacade *self, const char *proxyName) {
    const struct Facade *this = (struct Facade *) self;
    if (this->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::hasProxy] Error: Model unavailable to check registration for Proxy '%s' — skipping registration check.\033[0m\n", proxyName);
        return false;
    }
    return this->model->hasProxy(this->model, proxyName);
}

static bool removeProxy(const struct IFacade *self, const char *proxyName, struct IProxy **out) {
    const struct Facade *this = (struct Facade *) self;
    if (this->model == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeProxy] Error: Model unavailable to remove Proxy '%s' — skipping removal.\033[0m\n", proxyName);
        return false;
    }
    return this->model->removeProxy(this->model, proxyName, out);
}

static bool registerMediator(const struct IFacade *self, struct IMediator *(*factory)(void *buffer, const char *mediatorName, void *component), const char *mediatorName, void *component) {
    const struct Facade *this = (struct Facade *) self;
    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::registerMediator] Error: View unavailable to register Mediator '%s' — skipping registration.\033[0m\n", mediatorName);
        return false;
    }
    return this->view->registerMediator(this->view, factory, mediatorName, component);
}

static struct IMediator *retrieveMediator(const struct IFacade *self, const char *mediatorName) {
    const struct Facade *this = (struct Facade *) self;
    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::retrieveMediator] Error: View unavailable to retrieve Mediator '%s' — skipping retrieval.\033[0m\n", mediatorName);
        return false;
    }
    return this->view->retrieveMediator(this->view, mediatorName);
}

static bool hasMediator(const struct IFacade *self, const char *mediatorName) {
    const struct Facade *this = (struct Facade *) self;
    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::hasMediator] Error: View unavailable to check registration for Mediator '%s' — skipping registration check.\033[0m\n", mediatorName);
        return false;
    }
    return this->view->hasMediator(this->view, mediatorName);
}

static bool removeMediator(const struct IFacade *self, const char *mediatorName, struct IMediator **out) {
    const struct Facade *this = (struct Facade *) self;
    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeMediator] Error: View unavailable to remove Mediator '%s' — skipping removal.\033[0m\n", mediatorName);
        return false;
    }
    return this->view->removeMediator(this->view, mediatorName, out);
}

static void notifyObservers(const struct IFacade *self, struct INotification *notification) {
    const struct Facade *this = (struct Facade *) self;
    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::notifyObservers] Error: View unavailable to notify Observers '%s' — skipping notification.\033[0m\n", notification->getName(notification));
        return;
    }
    this->view->notifyObservers(this->view, notification);
}

static void sendNotification(const struct IFacade *self, const char *notificationName, void *body, const char *type) {
    struct INotification *notification = puremvc_notification_init(alloca(puremvc_notification_size()), notificationName, body, type);
    self->notifyObservers(self, notification);
}

size_t puremvc_facade_size(void) {
    return (sizeof(struct Facade) + (sizeof(void *) - 1u)) & ~(sizeof(void *) - 1u);
}

struct IFacade *puremvc_facade_init(void *buffer, const char *key) {
    if (buffer == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::init] Error: Buffer is NULL for Facade '%s' - skipping initialization.\033[0m\n", key != NULL ? key : "(unnamed)");
        return NULL;
    }

    struct Facade *this = buffer;
    memset(this, 0, sizeof(struct Facade));

    this->super.initializeFacade = initializeFacade;
    this->super.initializeController = initializeController;
    this->super.initializeModel = initializeModel;
    this->super.initializeView = initializeView;
    this->super.registerCommand = registerCommand;
    this->super.hasCommand = hasCommand;
    this->super.removeCommand = removeCommand;
    this->super.registerProxy = registerProxy;
    this->super.retrieveProxy = retrieveProxy;
    this->super.hasProxy = hasProxy;
    this->super.removeProxy = removeProxy;
    this->super.registerMediator = registerMediator;
    this->super.retrieveMediator = retrieveMediator;
    this->super.hasMediator = hasMediator;
    this->super.removeMediator = removeMediator;
    this->super.notifyObservers = notifyObservers;
    this->super.sendNotification = sendNotification;

    int len = snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    if (len < 0 || len >= KEY_SIZE) {
        memset(this, 0, sizeof(struct Facade));
        fprintf(stderr, "\033[0;31m[PureMVC::facade::init] Error: Facade multitonKey truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        return NULL;
    }

    return (struct IFacade *) this;
}

static void dispatchOnce(void) {
    mutex_init(&instanceMapMutex);
}

struct IFacade *puremvc_facade_getInstance(struct FacadeMap **facadeMap, const char *key) {
    if (key == NULL) return NULL;
    struct IFacade *facade = NULL;

    if (facadeMap != NULL) instanceMap = facadeMap; // exception for notifier

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t i = 0;
    for (; instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find facade
        if (strcmp(instanceMap[i]->key, key) == 0) {
            facade = instanceMap[i]->facade;
            goto finally;
        }
    }

    if (instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] ERROR: FacadeMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        goto finally;
    }

    if (instanceMap[i]->facade == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] ERROR: Missing Facade storage; skipping registration.\033[0m\n");
        goto finally;
    }

    int len = snprintf(instanceMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len < 0 || len >= KEY_SIZE) { // todo reset proxy or init proxy after, you have the name
        fprintf(stderr, "\033[0;31m[PureMVC::Model::getInstance] Error: ModelMap key truncated: '%s' (max %zu chars).\033[0m\n", key, sizeof(key));
        memset(instanceMap[i]->key, 0, KEY_SIZE);
        goto finally;
    }

    facade = puremvc_facade_init(instanceMap[i]->facade, key);

finally:
    mutex_unlock(&instanceMapMutex);
    return facade;
}

bool puremvc_facade_hasCore(const char *key) {
    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::hasFacade] ERROR: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return false;
    }

    bool exists = false;

    mutex_lock_shared(&instanceMapMutex);
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) {
        if (strcmp(instanceMap[i]->key, key) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&instanceMapMutex);

    return exists;
}

bool puremvc_facade_removeFacade(const char *key, struct IFacade **out) {
    if (key == NULL) return false;
    bool removed = false;

    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeFacade] ERROR: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return false;
    }

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t index = 0;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find facade
        if (strcmp(instanceMap[i]->key, key) == 0) {
            if (out != NULL) *out = instanceMap[i]->facade;
            puremvc_model_removeModel(key, NULL);
            puremvc_view_removeView(key, NULL);
            puremvc_controller_removeController(key, NULL);
            memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i) { // shift left (Gap-free array)
                *instanceMap[index] = *instanceMap[i]; // shift left first
            }
            index++;
        }
    }
    memset(instanceMap[index]->key, 0, KEY_SIZE); // reset tail slot

    if (index == 0) instanceMap = NULL; // avoid dangling global stack pointer after removal of last entry

    mutex_unlock(&instanceMapMutex);
    return removed;
}
