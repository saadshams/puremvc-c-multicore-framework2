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

// mutex for facadeMap
static Mutex facadeMapMutex;
static MutexOnce facadeMutexOnce = MUTEX_ONCE_INIT;

static void initializeFacade(struct IFacade *self, struct FacadeMap **facadeMap) {
    self->initializeModel(self, facadeMap->modelMap);
    self->initializeController(self, facadeMap->controllerMap);
    self->initializeView(self, facadeMap->viewMap);
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

static void registerCommand(const struct IFacade *self, const char *notificationName, struct SimpleCommand(*factory)()) {
    const struct Facade *this = (struct Facade *) self;
    this->controller->registerCommand(this->controller, notificationName, factory);
}

static bool hasCommand(const struct IFacade *self, const char *notificationName) {
    const struct Facade *this = (struct Facade *) self;
    return this->controller->hasCommand(this->controller, notificationName);
}

static void removeCommand(const struct IFacade *self, const char *notificationName) {
    const struct Facade *this = (struct Facade *) self;
    this->controller->removeCommand(this->controller, notificationName);
}

static void registerProxy(const struct IFacade *self, struct Proxy proxy) {
    const struct Facade *this = (struct Facade *) self;
    this->model->registerProxy(this->model, proxy);
}

static struct IProxy *retrieveProxy(const struct IFacade *self, const char *proxyName) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->retrieveProxy(this->model, proxyName);
}

static bool hasProxy(const struct IFacade *self, const char *proxyName) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->hasProxy(this->model, proxyName);
}

static struct Proxy removeProxy(const struct IFacade *self, const char *proxyName) {
    const struct Facade *this = (struct Facade *) self;
    return this->model->removeProxy(this->model, proxyName);
}

static void registerMediator(const struct IFacade *self, const struct Mediator mediator) {
    const struct Facade *this = (struct Facade *) self;
    this->view->registerMediator(this->view, mediator);
}

static struct IMediator *retrieveMediator(const struct IFacade *self, const char *mediatorName) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->retrieveMediator(this->view, mediatorName);
}

static bool hasMediator(const struct IFacade *self, const char *mediatorName) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->hasMediator(this->view, mediatorName);
}

static struct Mediator removeMediator(const struct IFacade *self, const char *mediatorName) {
    const struct Facade *this = (struct Facade *) self;
    return this->view->removeMediator(this->view, mediatorName);
}

static void notifyObservers(const struct IFacade *self, struct INotification *notification) {
    const struct Facade *this = (struct Facade *) self;
    this->view->notifyObservers(this->view, notification);
}

static void sendNotification(const struct IFacade *self, const char *notificationName, void *body, const char *type) {
    struct Notification notification = puremvc_notification(notificationName, body, type);
    self->notifyObservers(self, &notification.base);
}

static void init(struct Facade *facade, const char *key) {
    facade->base = (struct IFacade) {
        .initializeFacade = initializeFacade,
        .initializeController = initializeController,
        .initializeModel = initializeModel,
        .initializeView = initializeView,
        .registerCommand = registerCommand,
        .hasCommand = hasCommand,
        .removeCommand = removeCommand,
        .registerProxy = registerProxy,
        .retrieveProxy = retrieveProxy,
        .hasProxy = hasProxy,
        .removeProxy = removeProxy,
        .registerMediator = registerMediator,
        .retrieveMediator = retrieveMediator,
        .hasMediator = hasMediator,
        .removeMediator = removeMediator,
        .notifyObservers = notifyObservers,
        .sendNotification = sendNotification
    };

    snprintf(facade->multitonKey, KEY_SIZE, "%s", key);
}

static void deinit(struct Facade *facade) {
    memset(facade->multitonKey, 0, KEY_SIZE);
    facade->base = (struct IFacade){0}; // todo
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

    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    size_t i = 0;
    for (; facadeMap[i] != NULL && facadeMap[i]->key[0] != '\0'; i++) { // find model
        if (strncmp(facadeMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&facadeMapMutex);
            return &facadeMap[i]->facade.base;
        }
    }

    if (facadeMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::getInstance] FATAL: FacadeMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&facadeMapMutex);
        return NULL;
    }

    snprintf(facadeMap[i]->key, KEY_SIZE, "%s", key); // init
    init(&facadeMap[i]->facade, key);
    facadeMap[i]->facade.base.initializeFacade(facadeMap, &facadeMap[i]->facade.base);

    mutex_unlock(&facadeMapMutex);
    return &facadeMap[i]->facade.base;
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

void puremvc_facade_removeFacade(struct FacadeMap **facadeMap, const char *key) {
    if (facadeMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeFacade] FATAL: Missing FacadeMap storage; skipping registration.\033[0m\n");
        return;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Facade::removeFacade] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Model::removeFacade] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return;
    }

    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    puremvc_model_removeModel(facadeMap->modelMap, key);
    puremvc_view_removeView(facadeMap->viewMap, key);
    puremvc_controller_removeController(facadeMap->controllerMap, key);

    size_t index = 0;
    for (size_t i = 0; facadeMap[i] != NULL && facadeMap[i]->key[0] != '\0'; i++) { // find facade
        if (strncmp(facadeMap[i]->key, key, KEY_SIZE) == 0) {
            memset(facadeMap[i]->key, 0, KEY_SIZE); // clear model
            deinit(&facadeMap[i]->facade);
        } else {
            if (index != i) { // shift left
                snprintf(facadeMap[index]->key, KEY_SIZE, "%s", facadeMap[i]->key); // copy model (destination)
                facadeMap[index]->facade = facadeMap[i]->facade;

                memset(facadeMap[i]->key, 0, KEY_SIZE); // clear facade (source)
                deinit(&facadeMap[i]->facade);
            }
            index++;
        }
    }
    mutex_unlock(&facadeMapMutex);
}
