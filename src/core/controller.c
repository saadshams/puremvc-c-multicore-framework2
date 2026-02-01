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
#include "puremvc/mutex.h"

// The Multiton controllerMap.
static struct ControllerMap {
    char key[KEY_SIZE];
    struct Controller controller;
} controllerMap[INSTANCE_MAP_SIZE];

// mutex for controllerMap
static Mutex controllerMapMutex;
static MutexOnce controllerMutexOnce = MUTEX_ONCE_INIT;

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
            mutex_unlock(&self->commandMapMutex);
            return;
        }
    }

    if (i >= COMMAND_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Controller::registerCommand] Warning: CommandMap is at capacity for notification '%s' (max %d commands); skipping registration.\n", notificationName, COMMAND_MAP_SIZE);
        mutex_unlock(&self->commandMapMutex);
        return;
    }

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
    mutex_unlock(&self->commandMapMutex);
    return exists;
}

static void removeCommand(struct Controller *self, const char *notificationName) {
    mutex_lock(&self->commandMapMutex);
    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; i < COMMAND_MAP_SIZE && self->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(self->commandMap[i].key, notificationName) == 0) {
            self->view->removeObserver(self->view, notificationName, self);
            memset(&self->commandMap[index], 0, sizeof(struct CommandMap));
        } else {
            if (index != i) { // shift left
                self->view->removeObserver(self->view, notificationName, self); // remove before the shift

                memmove(&self->commandMap[index], &self->commandMap[i], sizeof(struct CommandMap));

                const struct Observer observer = puremvc_observer((void (*)(const void *, struct Notification)) executeCommand, self);
                self->view->registerObserver(self->view, notificationName, observer); // register after the shift

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

static void dispatchOnce(void) {
    mutex_init(&controllerMapMutex);
}

struct Controller *puremvc_controller_getInstance(const char *key, struct Controller(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t i = 0;
    for (; i < INSTANCE_MAP_SIZE && controllerMap[i].key[0] != '\0'; i++) {
        if (strncmp(controllerMap[i].key, key, KEY_SIZE) == 0) {
            mutex_unlock(&controllerMapMutex);
            return &controllerMap[i].controller;
        }
    }

    if (i >= INSTANCE_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Controller::getInstance] Warning: InstanceMap is at capacity for key '%s' (max %d instances); skipping registration.\n", key, INSTANCE_MAP_SIZE);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    snprintf(controllerMap[i].key, KEY_SIZE, "%s", key);
    controllerMap[i].controller = factory(key);
    mutex_init(&controllerMap[i].controller.commandMapMutex);

    controllerMap[i].controller.initializeController(&controllerMap[i].controller);

    mutex_unlock(&controllerMapMutex);
    return &controllerMap[i].controller;
}

void puremvc_controller_removeController(const char *key) {
    if (key == NULL) return;
    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && controllerMap[i].key[0] != '\0'; i++) {
        if (strcmp(controllerMap[i].key, key) == 0) {
            memset(&controllerMap[i], 0, sizeof(struct ControllerMap));
        } else {
            if (index != i) {
                memmove(&controllerMap[index], &controllerMap[i], sizeof(struct ControllerMap));
                memset(&controllerMap[i], 0, sizeof(struct ControllerMap));
            }
            index++;
        }
    }
    mutex_unlock(&controllerMapMutex);
}
