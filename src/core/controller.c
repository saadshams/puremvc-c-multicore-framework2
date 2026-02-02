/**
* @file Controller.c
* @internal
* @brief Controller Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/mutex.h"
#include "puremvc/controller.h"
#include "puremvc/view.h"

#include <stdio.h>
#include <string.h>

// The Multiton controllerMap.
static struct IControllerMap {
    char key[KEY_SIZE];
    struct Controller controller;
} controllerMap[INSTANCE_MAP_SIZE];

// mutex for controllerMap
static Mutex controllerMapMutex;
static MutexOnce controllerMutexOnce = MUTEX_ONCE_INIT;

static void initializeController(const struct IController *self) {
    struct Controller *this = (struct Controller *) self;
    if (this->view != NULL) return;
    this->view = puremvc_view_getInstance(this->multitonKey, puremvc_view);
}

static void executeCommand(const struct IController *self, struct INotification *notification) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock_shared(&this->commandMapMutex);
    for (size_t i = 0; i < COMMAND_MAP_SIZE && this->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i].key, notification->getName(notification)) == 0) {
            struct SimpleCommand (*factory)() = this->commandMap[i].factory;
            struct SimpleCommand command = factory();
            command.notifier.base.initializeNotifier(&command.notifier.base, this->multitonKey);
            command.base.execute(&command.base, notification);
            break;
        }
    }
    mutex_unlock(&this->commandMapMutex);
}

static void registerCommand(struct IController *self, const char *notificationName, struct SimpleCommand(*factory)()) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock(&this->commandMapMutex);
    size_t i = 0;
    for (; i < COMMAND_MAP_SIZE && this->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i].key, notificationName) == 0) {
            this->commandMap[i].factory = factory;
            mutex_unlock(&this->commandMapMutex);
            return;
        }
    }

    if (i >= COMMAND_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Controller::registerCommand] Warning: CommandMap is at capacity for notification '%s' (max %d commands); skipping registration.\n", notificationName, COMMAND_MAP_SIZE);
        mutex_unlock(&this->commandMapMutex);
        return;
    }

    const struct Observer observer = puremvc_observer((void (*)(const void *, struct INotification *)) executeCommand, self);
    this->view->registerObserver(this->view, notificationName, observer);

    int len = snprintf(this->commandMap[i].key, KEY_SIZE, "%s", notificationName);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Controller::registerCommand] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", notificationName, len, KEY_SIZE);

    this->commandMap[i].factory = factory;
    mutex_unlock(&this->commandMapMutex);
}

static bool hasCommand(const struct IController *self, const char *notificationName) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock_shared(&this->commandMapMutex);
    bool exists = false;
    for (size_t i = 0; i < COMMAND_MAP_SIZE && this->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i].key, notificationName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->commandMapMutex);
    return exists;
}

static void removeCommand(struct IController *self, const char *notificationName) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock(&this->commandMapMutex);
    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; i < COMMAND_MAP_SIZE && this->commandMap[i].key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i].key, notificationName) == 0) {
            this->view->removeObserver(this->view, notificationName, self);
            memset(&this->commandMap[index], 0, sizeof(struct CommandMap));
        } else {
            if (index != i) { // shift left
                this->view->removeObserver(this->view, notificationName, self); // remove before the shift

                memmove(&this->commandMap[index], &this->commandMap[i], sizeof(struct CommandMap));

                const struct Observer observer = puremvc_observer((void (*)(const void *, struct INotification *)) executeCommand, self);
                this->view->registerObserver(this->view, notificationName, observer); // register after the shift

                memset(&this->commandMap[i], 0, sizeof(struct CommandMap));
            }
            index++;
        }
    }
    mutex_unlock(&this->commandMapMutex);
}

struct Controller puremvc_controller(const char *key) {
    struct Controller controller = {
        .base = {
            .initializeController = initializeController,
            .executeCommand = executeCommand,
            .registerCommand = registerCommand,
            .hasCommand = hasCommand,
            .removeCommand = removeCommand
        },
    };

    int len = snprintf(controller.multitonKey, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Controller] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    return controller;
}

static void dispatchOnce(void) {
    mutex_init(&controllerMapMutex);
}

struct IController *puremvc_controller_getInstance(const char *key, struct Controller(*factory)(const char *key)) {
    if (key == NULL || factory == NULL) return NULL;
    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t i = 0;
    for (; i < INSTANCE_MAP_SIZE && controllerMap[i].key[0] != '\0'; i++) {
        if (strncmp(controllerMap[i].key, key, KEY_SIZE) == 0) {
            mutex_unlock(&controllerMapMutex);
            return &controllerMap[i].controller.base;
        }
    }

    if (i >= INSTANCE_MAP_SIZE) {
        fprintf(stderr, "[PureMVC::Controller::getInstance] Warning: InstanceMap is at capacity for key '%s' (max %d instances); skipping registration.\n", key, INSTANCE_MAP_SIZE);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    int len = snprintf(controllerMap[i].key, KEY_SIZE, "%s", key);
    if (len >= KEY_SIZE)
        fprintf(stderr, "[PureMVC::Controller::getInstance] Warning: Key Truncated: '%s' (Original length: %d, Buffer size: %d)\n", key, len, KEY_SIZE);

    controllerMap[i].controller = factory(key);
    mutex_init(&controllerMap[i].controller.commandMapMutex);

    controllerMap[i].controller.base.initializeController(&controllerMap[i].controller.base);

    mutex_unlock(&controllerMapMutex);
    return &controllerMap[i].controller.base;
}

void puremvc_controller_removeController(const char *key) {
    if (key == NULL) return;
    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t index = 0;
    for (size_t i = 0; i < INSTANCE_MAP_SIZE && controllerMap[i].key[0] != '\0'; i++) {
        if (strcmp(controllerMap[i].key, key) == 0) {
            memset(&controllerMap[i], 0, sizeof(struct IControllerMap));
        } else {
            if (index != i) {
                memmove(&controllerMap[index], &controllerMap[i], sizeof(struct IControllerMap));
                memset(&controllerMap[i], 0, sizeof(struct IControllerMap));
            }
            index++;
        }
    }
    mutex_unlock(&controllerMapMutex);
}
