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

#include <puremvc/mutex.h>
#include "puremvc/facade.h"
#include "puremvc/controller.h"
#include "puremvc/model.h"
#include "puremvc/view.h"
#include "puremvc/notification.h"

// facadeMap
static struct FacadeMap **s_facadeMap = NULL;

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

static bool removeProxy(const struct IFacade *self, const char *proxyName, struct IProxy **proxy) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->removeProxy(this->model, proxyName, proxy);
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

static bool removeMediator(const struct IFacade *self, const char *mediatorName, struct IMediator **mediator) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->removeMediator(this->view, mediatorName, mediator);
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

    snprintf(this->multitonKey, KEY_SIZE, "%s", key);
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

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Facade::getInstance] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return NULL;
    }

    if (s_facadeMap == NULL)
        s_facadeMap = facadeMap;

    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    size_t i = 0;
    for (; s_facadeMap[i] != NULL && s_facadeMap[i]->key[0] != '\0'; i++) { // find facade
        if (strncmp(s_facadeMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&facadeMapMutex);
            return s_facadeMap[i]->facade;
        }
    }

    if (s_facadeMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: FacadeMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&facadeMapMutex);
        return NULL;
    }

    snprintf(s_facadeMap[i]->key, KEY_SIZE, "%s", key); // init
    puremvc_facade_init(facadeMap[i]->facade, key);
    s_facadeMap[i]->facade->initializeFacade(s_facadeMap[i]->facade, facadeMap);

    mutex_unlock(&facadeMapMutex);
    return s_facadeMap[i]->facade;
}

bool puremvc_facade_hasCore(struct FacadeMap **facadeMap, const char *key) {
    mutex_lock_shared(&facadeMapMutex);
    bool exists = false;
    for (size_t i = 0; facadeMap != NULL && facadeMap[i]->key[0] != '\0'; i++) {
        if (strcmp(facadeMap[i]->key, key) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&facadeMapMutex);
    return exists;
}

bool puremvc_facade_removeFacade(const char *key, struct IFacade **facade) {
    if (s_facadeMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeFacade] FATAL: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeFacade] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return false;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::removeFacade] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return false;
    }

    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    puremvc_model_removeModel(key, NULL);
    puremvc_view_removeView(key, NULL);
    puremvc_controller_removeController(key, NULL);

    size_t index = 0;
    for (size_t i = 0; s_facadeMap[i] != NULL && s_facadeMap[i]->key[0] != '\0'; i++) { // find facade
        if (strncmp(s_facadeMap[i]->key, key, KEY_SIZE) == 0) {
            memset(&s_facadeMap[i]->key, 0, KEY_SIZE); // clear model
            if (facade != NULL)
                *facade = s_facadeMap[i]->facade;
        } else {
            if (index != i) { // shift left
                *s_facadeMap[index] = *s_facadeMap[i];
                memset(s_facadeMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }

    if (index == 0) // all keys were removed; reset
        s_facadeMap = NULL;

    mutex_unlock(&facadeMapMutex);

    return true;
}
