#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "facade.h"
#include "dictionary.h"

// The Multiton Facade instanceMap.
static Dictionary *instanceMap;

// mutex for instanceMap
static pthread_rwlock_t instanceMap_mutex = PTHREAD_MUTEX_INITIALIZER;

static void initializeFacade(struct Facade *self) {
    self->initializeModel(self);
    self->initializeController(self);
    self->initializeView(self);
}

static void initializeController(struct Facade *self) {
    if (self->controller != NULL) return;
    self->controller = puremvc_controller_getInstance(self->multitonKey, puremvc_controller_new);
}

static void initializeModel(struct Facade *self) {
    if (self->model != NULL) return;
    self->model = puremvc_model_getInstance(self->multitonKey, puremvc_model_new);
}

static void initializeView(struct Facade *self) {
    if (self->view != NULL) return;
    self->view = puremvc_view_getInstance(self->multitonKey, puremvc_view_new);
}

static void registerCommand(struct Facade *self, const char *notificationName, struct SimpleCommand *(*factory)(void)) {
    self->controller->registerCommand(self->controller, notificationName, factory);
}

static void removeCommand(struct Facade *self, const char *notificationName) {
    self->controller->removeCommand(self->controller, notificationName);
}

static bool hasCommand(struct Facade *self, const char *notificationName) {
    return self->controller->hasCommand(self->controller, notificationName);
}

static void registerProxy(struct Facade *self, struct Proxy *proxy) {
    self->model->registerProxy(self->model, proxy);
}

static struct Proxy *retrieveProxy(struct Facade *self, const char *proxyName) {
    return self->model->retrieveProxy(self->model, proxyName);
}

static struct Proxy *removeProxy(struct Facade *self, const char *proxyName) {
    return self->model->removeProxy(self->model, proxyName);
}

static bool hasProxy(struct Facade *self, const char *proxyName) {
    return self->model->hasProxy(self->model, proxyName);
}

static void registerMediator(struct Facade *self, struct Mediator *mediator) {
    self->view->registerMediator(self->view, mediator);
}

static struct Mediator *retrieveMediator(struct Facade *self, const char *mediatorName) {
    return self->view->retrieveMediator(self->view, mediatorName);
}

static struct Mediator *removeMediator(struct Facade *self, const char *mediatorName) {
    return self->view->removeMediator(self->view, mediatorName);
}

static bool hasMediator(struct Facade *self, const char *mediatorName) {
    return self->view->hasMediator(self->view, mediatorName);
}

static void sendNotification(struct Facade *self, const char *notificationName, void *body, char *type) {
    struct Notification *notification = puremvc_notification_new(notificationName, body, type);
    self->notifyObservers(self, notification);
    puremvc_notification_free(&notification);
}

static void notifyObservers(struct Facade *self, struct Notification *notification) {
    self->view->notifyObservers(self->view, notification);
}

static void initializeNotifier(struct Facade *self, const char *key) {
    self->multitonKey = key;
}

struct Facade *puremvc_facade_init(struct Facade *self) {
    self->initializeFacade = initializeFacade;
    self->initializeController = initializeController;
    self->initializeModel = initializeModel;
    self->initializeView = initializeView;
    self->registerCommand = registerCommand;
    self->removeCommand = removeCommand;
    self->hasCommand = hasCommand;
    self->registerProxy = registerProxy;
    self->retrieveProxy = retrieveProxy;
    self->removeProxy = removeProxy;
    self->hasProxy = hasProxy;
    self->registerMediator = registerMediator;
    self->retrieveMediator = retrieveMediator;
    self->removeMediator = removeMediator;
    self->hasMediator = hasMediator;
    self->sendNotification = sendNotification;
    self->notifyObservers = notifyObservers;
    self->initializeNotifier = initializeNotifier;
    return self;
}

struct Facade *puremvc_facade_alloc(const char *key) {
    assert(instanceMap->get(instanceMap, key) == NULL);

    struct Facade *self = malloc(sizeof(struct Facade));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    puremvc_facade_init(self);
    self->multitonKey = strdup(key);
    self->controller = NULL;
    self->model = NULL;
    self->view = NULL;
    return self;

    exception:
    fprintf(stderr, "Facade allocation failed.\n");
    return NULL;
}

struct Facade *puremvc_facade_new(const char *key) {
    return puremvc_facade_init(puremvc_facade_alloc(key));
}

void puremvc_facade_free(struct Facade **self) {
    free((void *) (*self)->multitonKey);
    free(*self);
    *self = NULL;
}

struct Facade *puremvc_facade_getInstance(const char *key, struct Facade *(*factory)(const char *)) {
    pthread_rwlock_wrlock(&instanceMap_mutex);

    if (instanceMap == NULL) instanceMap = puremvc_dictionary_new();

    struct Facade *instance = (struct Facade *) instanceMap->get(instanceMap, key);
    if (instance == NULL) {
        instance = (struct Facade *) instanceMap->put(instanceMap, key, factory(key));
        instance->initializeFacade(instance);
    }
    pthread_rwlock_unlock(&instanceMap_mutex);
    return instance;
}

bool puremvc_facade_hasCore(const char *key) {
    pthread_rwlock_rdlock(&instanceMap_mutex);
    bool result = instanceMap->containsKey(instanceMap, key);
    pthread_rwlock_unlock(&instanceMap_mutex);
    return result;
}

void puremvc_facade_removeFacade(const char *key) {
    pthread_rwlock_wrlock(&instanceMap_mutex);
    puremvc_model_removeModel(key);
    puremvc_view_removeView(key);
    puremvc_controller_removeController(key);
    struct Facade *facade = (struct Facade *) instanceMap->removeItem(instanceMap, key);
    if (facade != NULL) puremvc_facade_free(&facade);
    pthread_rwlock_unlock(&instanceMap_mutex);
}
