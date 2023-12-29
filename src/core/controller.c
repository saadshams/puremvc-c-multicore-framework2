#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "controller.h"
#include "notifier.h"

// The Multiton Controller instanceMap.
static Dictionary *instanceMap;

// mutex for controller instanceMap
static pthread_rwlock_t instanceMap_mutex = PTHREAD_RWLOCK_INITIALIZER;

// mutex for commandMap
static pthread_rwlock_t commandMap_mutex = PTHREAD_RWLOCK_INITIALIZER;

static void initializeController(struct Controller *self) {
    self->view = puremvc_view_getInstance(self->multitonKey, puremvc_view_new);
}

static void executeCommand(struct Controller *self, struct Notification *notification) {
    pthread_rwlock_rdlock(&commandMap_mutex);

    struct SimpleCommand *(*factory)() = self->commandMap->get(self->commandMap, notification->name);
    if (factory) {
        struct SimpleCommand *commandInstance = factory();
        commandInstance->notifier->initializeNotifier(commandInstance->notifier, self->multitonKey);
        commandInstance->execute(commandInstance, notification);
        puremvc_simple_command_free(&commandInstance);
    }

    pthread_rwlock_unlock(&commandMap_mutex);
}

static void registerCommand(struct Controller *self, const char *notificationName, struct SimpleCommand *(factory)(void)) {
    pthread_rwlock_wrlock(&commandMap_mutex);
    if (self->commandMap->containsKey(self->commandMap, notificationName) == false) {
        struct Observer *observer = puremvc_observer_new((void (*)(void *, struct Notification *)) executeCommand, self);
        self->view->registerObserver(self->view, notificationName, observer);

        self->commandMap->put(self->commandMap, notificationName, factory);
    } else {
        self->commandMap->replace(self->commandMap, notificationName, factory);
    }
    pthread_rwlock_unlock(&commandMap_mutex);
}

static bool hasCommand(struct Controller *self, const char *notificationName) {
    pthread_rwlock_rdlock(&commandMap_mutex);
    bool exists = self->commandMap->containsKey(self->commandMap, notificationName);
    pthread_rwlock_unlock(&commandMap_mutex);
    return exists;
}

static void removeCommand(struct Controller *self, const char *notificationName) {
    pthread_rwlock_wrlock(&commandMap_mutex);
    if (self->commandMap->containsKey(self->commandMap, notificationName)) {
        self->commandMap->removeItem(self->commandMap, notificationName);

        // remove the observer
        self->view->removeObserver(self->view, notificationName, self);
    }
    pthread_rwlock_unlock(&commandMap_mutex);
}

struct Controller *puremvc_controller_init(struct Controller *self) {
    self->initializeController = initializeController;
    self->executeCommand = executeCommand;
    self->registerCommand = registerCommand;
    self->hasCommand = hasCommand;
    self->removeCommand = removeCommand;
    return self;
}

struct Controller *puremvc_controller_alloc(const char *key) {
    assert(instanceMap->get(instanceMap, key) == NULL);

    struct Controller *self = malloc(sizeof(struct Controller));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    puremvc_controller_init(self);
    self->multitonKey = strdup(key);
    self->commandMap = puremvc_dictionary_new();
    return self;

    exception:
    fprintf(stderr, "Controller allocation failed.\n");
    return NULL;
}

struct Controller *puremvc_controller_new(const char *key) {
    return puremvc_controller_init(puremvc_controller_alloc(key));
}

void puremvc_controller_free(struct Controller **self) {
    free((void *) (*self)->multitonKey);
    (*self)->commandMap->clear((*self)->commandMap, free);
    puremvc_dictionary_free(&(*self)->commandMap);

    free(*self);
    *self = NULL;
}

void puremvc_controller_removeController(const char *key) {
    pthread_rwlock_wrlock(&instanceMap_mutex);
    struct Controller *controller = (struct Controller *) instanceMap->removeItem(instanceMap, key);
    if (controller != NULL) puremvc_controller_free(&controller);
    pthread_rwlock_unlock(&instanceMap_mutex);
}

struct Controller *puremvc_controller_getInstance(const char *key, struct Controller *(*factory)(const char *)) {
    pthread_rwlock_wrlock(&instanceMap_mutex);

    if (instanceMap == NULL) instanceMap = puremvc_dictionary_new();

    struct Controller *instance = (struct Controller *) instanceMap->get(instanceMap, key);
    if (instance == NULL) {
        instance = (struct Controller *) instanceMap->put(instanceMap, key, factory(key));
        instance->initializeController(instance);
    }
    pthread_rwlock_unlock(&instanceMap_mutex);
    return instance;
}
