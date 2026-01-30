/**
* @file Controller.c
* @internal
* @brief Controller Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>
#include <string.h>

#include "puremvc/controller.h"

// The Multiton Controller instanceMap.
static struct Controller instanceMap[INSTANCE_MAP_SIZE];
static size_t instanceMapCount = 0;

// mutex for controller instanceMap
// static MutexOnce token = MUTEX_ONCE_INIT;
// static Mutex mutex;

static void initializeController(struct Controller *self) {
    if (self->view != NULL) return;
    self->view = puremvc_view_getInstance(self->multitonKey, puremvc_view);
}

static void executeCommand(const struct Controller *self, struct Notification notification) {
    // mutex_lock_shared(&this->commandMapMutex);
    for (size_t i = 0; i < self->commandMapCount; i++) {
        if (strcmp(self->commandMap[i].key, notification.name) == 0) {
            struct SimpleCommand (*factory)() = self->commandMap[i].factory;
            struct SimpleCommand command = factory();
            command.notifier.initializeNotifier(&command.notifier, self->multitonKey);
            command.execute(&command, notification);
        }
    }
    // mutex_unlock(&this->commandMapMutex);
}

static void registerCommand(struct Controller *self, const char *notificationName, struct SimpleCommand(*factory)()) {
    // mutex_lock(&this->commandMapMutex);

    for (size_t i = 0; i < self->commandMapCount; i++) {
        if (strcmp(self->commandMap[i].key, notificationName) == 0) { // search
            self->commandMap[i].factory = factory; // replace
            return;
        }
    }

    const struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification))executeCommand, self);
    self->view->registerObserver(self->view, notificationName, observer);

    snprintf(self->commandMap[self->commandMapCount].key, NAME_SIZE, "%s", notificationName);
    self->commandMap[self->commandMapCount].factory = factory;
    self->commandMapCount++;
    // mutex_unlock(&this->commandMapMutex);
}

static bool hasCommand(const struct Controller *self, const char *notificationName) {
    // mutex_lock_shared(&self->commandMapMutex);
    bool exists = false;
    for (size_t i = 0; i < self->commandMapCount; i++) {
        if (strcmp(self->commandMap[i].key, notificationName) == 0) {
            exists = true;
            break;
        }
    }
    // mutex_unlock(&self->commandMapMutex);
    return exists;
}

static void removeCommand(struct Controller *self, const char *notificationName) {
    // mutex_lock(&self->commandMapMutex);
    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; i < self->commandMapCount; i++) {
        if (strcmp(self->commandMap[i].key, notificationName) == 0) {
            self->view->removeObserver(self->view, notificationName, self);
            memset(&self->commandMap, 0, sizeof(struct CommandMap));
            self->commandMapCount--;
        } else {
            if (index != i) { // shift left
                memmove(&self->commandMap[index], &self->commandMap[i], sizeof(struct SimpleCommand));
                memset(&self->commandMap[i], 0, sizeof(struct SimpleCommand));
            }
            index++;
        }
    }
    // mutex_unlock(&self->commandMapMutex);
}

struct Controller puremvc_controller(const char *key) {
    struct Controller controller = {0};

    snprintf(controller.multitonKey, KEY_SIZE, "%s", key);

    controller.initializeController = initializeController;
    controller.executeCommand = executeCommand;
    controller.registerCommand = registerCommand;
    controller.hasCommand = hasCommand;
    controller.removeCommand = removeCommand;

    return controller;
}

static void dispatchOnce() {
    // mutex_init(&mutex);
}

struct Controller *puremvc_controller_getInstance(const char *key, struct Controller(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    for (size_t i = 0; i < instanceMapCount; i++) { // get
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            return &instanceMap[i];
        }
    }

    if (instanceMapCount >= INSTANCE_MAP_SIZE) return NULL;

    instanceMap[instanceMapCount] = factory(key);

    // mutex_unlock(&mutex);
    return &instanceMap[instanceMapCount++];
}

void puremvc_controller_removeController(const char *key) {
    if (key == NULL) return;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    for (size_t i = 0; i < instanceMapCount; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            memset(&instanceMap[i], 0, sizeof(struct Controller)); // remove

            for (size_t j = i + 1; j < instanceMapCount; j++) // shift left
                instanceMap[j-1] = instanceMap[j];

            instanceMapCount--;
            break;
        }
    }
    // mutex_unlock(&mutex);
}
