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

// The Multiton facadeMap.
static struct FacadeMap {
    char key[KEY_SIZE];
    struct Facade facade;
} facadeMap[INSTANCE_MAP_SIZE];

// mutex for facadeMap
static Mutex facadeMapMutex;
static MutexOnce facadeMutexOnce = MUTEX_ONCE_INIT;

static void initializeFacade(struct IFacade *self) {
    self->initializeModel(self);
    self->initializeController(self);
    self->initializeView(self);
}

static void initializeController(struct IFacade *self) {
    struct Facade *this = (struct Facade *) self;
    if (this->controller != NULL) return;
    this->controller = puremvc_controller_getInstance(this->multitonKey, puremvc_controller);
    this->controller->initializeController(this->controller);
}

static void initializeModel(struct IFacade *self) {
    struct Facade *this = (struct Facade *) self;
    if (this->model != NULL) return;
    this->model = puremvc_model_getInstance(this->multitonKey, puremvc_model);
    this->model->initializeModel(this->model);
}

static void initializeView(struct IFacade *self) {
    struct Facade *this = (struct Facade *) self;
    if (this->view != NULL) return;
    this->view = puremvc_view_getInstance(this->multitonKey, puremvc_view);
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

struct Facade puremvc_facade(const char *key) {
    struct Facade facade = {
        .base = {
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
        },
    };

    int len = snprintf(facade.multitonKey, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Facade] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    return facade;
}

static void dispatchOnce(void) {
    mutex_init(&facadeMapMutex);
}

struct IFacade *puremvc_facade_getInstance(const char *key, struct Facade(*factory)(const char *)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    size_t i = 0;
    for (; facadeMap[i].key[0] != '\0'; i++) {
        if (strncmp(facadeMap[i].key, key, KEY_SIZE) == 0) {
            mutex_unlock(&facadeMapMutex);
            return &facadeMap[i].facade.base;
        }
    }

    if (i >= INSTANCE_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Facade::getInstance] Warning: facadeMap is at capacity for key '%s' (max %d instances); skipping registration.\n", key, INSTANCE_MAP_SIZE);
        mutex_unlock(&facadeMapMutex);
        return NULL;
    }

    int len = snprintf(facadeMap[i].key, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Facade::getInstance] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    facadeMap[i].facade = factory(key);

    facadeMap[i].facade.base.initializeFacade(&facadeMap[i].facade.base);

    mutex_unlock(&facadeMapMutex);
    return &facadeMap[i].facade.base;
}

bool puremvc_facade_hasCore(const char *key) {
    if (key == NULL) return false;
    mutex_lock_shared(&facadeMapMutex);
    bool exists = false;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && facadeMap[i].key[0] != '\0'; i++) {
        if (strcmp(facadeMap[i].key, key) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&facadeMapMutex);
    return exists;
}

void puremvc_facade_removeFacade(const char *key) {
    if (key == NULL) return;
    mutex_once(&facadeMutexOnce, dispatchOnce);
    mutex_lock(&facadeMapMutex);

    puremvc_model_removeModel(key);
    puremvc_view_removeView(key);
    puremvc_controller_removeController(key);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && facadeMap[i].key[0] != '\0'; i++) {
        if (strcmp(facadeMap[i].key, key) == 0) {
            memset(&facadeMap[i], 0, sizeof(struct FacadeMap));
        } else {
            if (index != i) {
                memmove(&facadeMap[index], &facadeMap[i], sizeof(struct FacadeMap));
                memset(&facadeMap[i], 0, sizeof(struct FacadeMap));
            }
            index++;
        }
    }
    mutex_unlock(&facadeMapMutex);
}
