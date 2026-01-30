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
static MutexOnce token = MUTEX_ONCE_INIT;
static Mutex mutex;

static void initializeController(struct Controller *self) {
    if (self->view != NULL) return;
    self->view = puremvc_view_getInstance(self->multitonKey, puremvc_view);
}

static void executeCommand(struct Controller *self, const struct Notification notification) {
    mutex_lock_shared(&self->commandMapMutex);
    for (size_t i = 0; i < COMMAND_MAP_SIZE && self->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->commandMap[i].key, notification.name) == 0) {
            struct SimpleCommand (*factory)() = self->commandMap[i].factory;
            struct SimpleCommand command = factory();
            command.notifier.initializeNotifier(&command.notifier, self->multitonKey);
            command.execute(&command, notification);
            break;
        }
    }
    mutex_unlock(&self->commandMapMutex);
}

static void registerCommand(struct Controller *self, const char *notificationName, struct SimpleCommand(*factory)()) {
    mutex_lock(&self->commandMapMutex);
    size_t i = 0;
    for (; i < COMMAND_MAP_SIZE && self->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->commandMap[i].key, notificationName) == 0) {
            self->commandMap[i].factory = factory;
            return mutex_unlock(&self->commandMapMutex), (void)0;
        }
    }

    if (i >= COMMAND_MAP_SIZE) return mutex_unlock(&self->commandMapMutex), (void)0; // commandMap is full

    const struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) executeCommand, self);
    self->view->registerObserver(self->view, notificationName, observer);

    snprintf(self->commandMap[i].key, NAME_SIZE, "%s", notificationName);
    self->commandMap[i].factory = factory;
    mutex_unlock(&self->commandMapMutex);
}

static bool hasCommand(struct Controller *self, const char *notificationName) {
    mutex_lock_shared(&self->commandMapMutex);
    bool exists = false;
    for (size_t i = 0; i < COMMAND_MAP_SIZE && self->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->commandMap[i].key, notificationName) == 0) {
            exists = true;
            break;
        }
    }
    return mutex_unlock(&self->commandMapMutex), exists;
}

static void removeCommand(struct Controller *self, const char *notificationName) {
    mutex_lock(&self->commandMapMutex);
    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; i < COMMAND_MAP_SIZE && i < self->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->commandMap[i].key, notificationName) == 0) {
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
    mutex_unlock(&self->commandMapMutex);
}

struct Controller puremvc_controller(const char *key) {
    struct Controller controller = {
        .initializeController = initializeController,
        .executeCommand = executeCommand,
        .registerCommand = registerCommand,
        .hasCommand = hasCommand,
        .removeCommand = removeCommand
    };

    snprintf(controller.multitonKey, KEY_SIZE, "%s", key);
    return controller;
}

static void dispatchOnce() {
    mutex_init(&mutex);
}

struct Controller *puremvc_controller_getInstance(const char *key, struct Controller(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&mutex);

    size_t i = 0;
    for (; instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strncmp(instanceMap[i].multitonKey, key, KEY_SIZE) == 0) {
            return mutex_unlock(&mutex), &instanceMap[i];
        }
    }

    if (i >= INSTANCE_MAP_SIZE) return mutex_unlock(&mutex), NULL;

    instanceMap[i] = factory(key);

    return mutex_unlock(&mutex), &instanceMap[i];
}

void puremvc_controller_removeController(const char *key) {
    if (key == NULL) return;
    mutex_once(&token, dispatchOnce);
    mutex_lock(&mutex);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && instanceMap[i].multitonKey[0] != '\0'; i++) {
        if (strcmp(instanceMap[i].multitonKey, key) != 0) {
            if (index != i)
                memmove(&instanceMap[index], &instanceMap[i], sizeof(struct Controller));
            index++;
        }
    }
    memset(&instanceMap[index], 0, sizeof(struct Controller));
    mutex_unlock(&mutex);
}
