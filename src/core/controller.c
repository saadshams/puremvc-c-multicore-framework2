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

// mutex for controller instanceMap
// static MutexOnce token = MUTEX_ONCE_INIT;
// static Mutex mutex;

static void initializeController(struct Controller *self) {
    if (self->view != NULL) return;
    self->view = puremvc_view_getInstance(self->multitonKey, puremvc_view);
}

static void executeCommand(const struct Controller *self, const struct Notification notification) {
    // mutex_lock_shared(&this->commandMapMutex);
    for (size_t i = 0; i < COMMAND_MAP_SIZE && self->commandMap[i].key[0] != '\0'; i++) { // search
        if (strcmp(self->commandMap[i].key, notification.name) == 0) { // match
            struct SimpleCommand (*factory)() = self->commandMap[i].factory;
            struct SimpleCommand command = factory();
            command.notifier.initializeNotifier(&command.notifier, self->multitonKey);
            command.execute(&command, notification);
            break;
        }
    }
    // mutex_unlock(&this->commandMapMutex);
}

static void registerCommand(struct Controller *self, const char *notificationName, struct SimpleCommand(*factory)()) {
    // mutex_lock(&this->commandMapMutex);
    size_t i = 0;
    for (; i < COMMAND_MAP_SIZE && self->commandMap[i].key[0] != '\0'; i++) { // search
        if (strcmp(self->commandMap[i].key, notificationName) == 0) { // match
            self->commandMap[i].factory = factory; // update
            return;
        }
    }

    if (i >= COMMAND_MAP_SIZE) return; // commandMap is full

    const struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) executeCommand, self);
    self->view->registerObserver(self->view, notificationName, observer);

    snprintf(self->commandMap[i].key, NAME_SIZE, "%s", notificationName); // new key
    self->commandMap[i].factory = factory; // insert
    // mutex_unlock(&this->commandMapMutex);
}

static bool hasCommand(const struct Controller *self, const char *notificationName) {
    // mutex_lock_shared(&self->commandMapMutex);
    bool exists = false;
    for (size_t i = 0; i < COMMAND_MAP_SIZE && self->commandMap[i].key[0] != '\0'; i++) {
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
    for (size_t i = 0; i < COMMAND_MAP_SIZE && i < self->commandMap[i].key[0] != '\0'; i++) { // search
        if (strcmp(self->commandMap[i].key, notificationName) == 0) { // match
            self->view->removeObserver(self->view, notificationName, self);
            memset(&self->commandMap, 0, sizeof(struct CommandMap));
        } else {
            if (index != i) { // shift left
                memmove(&self->commandMap[index], &self->commandMap[i], sizeof(struct SimpleCommand));
                memset(&self->commandMap[i], 0, sizeof(struct CommandMap));
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

void puremvc_controller_removeController(const char *key) {
    if (key == NULL) return;
    // mutex_once(&token, dispatchOnce);
    // mutex_lock(&mutex);

    for (size_t i = 0; i < INSTANCE_MAP_SIZE; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) == 0) {
            memset(&instanceMap[i], 0, sizeof(struct Controller)); // remove

            for (size_t j = i + 1; j < INSTANCE_MAP_SIZE; j++) // shift left
                // instanceMap[j-1] = instanceMap[j];
                memmove(&instanceMap[j], &instanceMap[j + 1], sizeof(struct Controller));
            break;
        }
    }
    // mutex_unlock(&mutex);
}
