/**
* @file Controller.c
* @internal
* @brief Controller Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#include "controller.h"

#include "puremvc/i_mutex.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_command.h"

#include <alloca.h>
#include <stdio.h>
#include <string.h>

// instanceMap
static struct ControllerMap **instanceMap = NULL;

// mutex for controllerMap
static Mutex controllerMapMutex;
static MutexOnce controllerMutexOnce = MUTEX_ONCE_INIT;

static void initializeController(struct IController *self, struct IView *view, struct CommandMap **commandMap) {
    struct Controller *this = (struct Controller *) self;
    if (commandMap != NULL) this->commandMap = commandMap;
    if (this->view != NULL || view == NULL) return;
    this->view = view;
}

static bool registerCommand(struct IController *self, const char *notificationName, struct ICommand *(*factory)(void *buffer)) {
    struct Controller *this = (struct Controller *) self;

    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[[PureMVC::Controller::registerCommand] Error: View unavailable to register Command '%s' — skipping registration.\033[0m\n", notificationName);
        return false;
    }

    if (this->commandMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] FATAL: Missing CommandMap field in ControllerMap; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock(&this->commandMapMutex);

    size_t i = 0;
    for (; this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) { // existing
        if (this->commandMap[i]->key == notificationName || strcmp(this->commandMap[i]->key, notificationName) == 0) {
            fprintf(stderr, "\033[0;33m[PureMVC::Controller::registerCommand] Warning: Command '%s' exists; overriding registration.\033[0m\n", notificationName);

            this->commandMap[i]->factory = factory; // registration
            mutex_unlock(&this->commandMapMutex);
            return true;
        }
    }

    if (this->commandMap[i] == NULL) { // overflow (CommandMap)
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] Error: CommandMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    if (this->view->registerObserver(this->view, notificationName, (void (*)(const void *, const struct INotification *)) self->executeCommand, self)) {
        this->commandMap[i]->key = notificationName; // registration
        this->commandMap[i]->factory = factory;
        mutex_unlock(&this->commandMapMutex);
        return true;
    }

    return false;
}

static void executeCommand(const struct IController *self, struct INotification *notification) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock_shared(&this->commandMapMutex);

    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) {
        if (this->commandMap[i]->key == notification->getName(notification) || strcmp(this->commandMap[i]->key, notification->getName(notification)) == 0) {
            struct ICommand *(*factory)(void *) = this->commandMap[i]->factory;
            const struct ICommand *command = factory(alloca(puremvc_simple_command_size()));
            command->getNotifier(command)->initializeNotifier(command->getNotifier(command), this->multitonKey);
            command->execute(command, notification);
            break;
        }
    }

    mutex_unlock(&this->commandMapMutex);
}

static bool hasCommand(const struct IController *self, const char *notificationName) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock_shared(&this->commandMapMutex);
    bool exists = false;
    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) {
        if (this->commandMap[i]->key == notificationName || strcmp(this->commandMap[i]->key, notificationName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->commandMapMutex);
    return exists;
}

static bool removeCommand(struct IController *self, const char *notificationName, struct ICommand *(**out)(void *buffer)) {
    struct Controller *this = (struct Controller *) self;
    bool removed = false;

    mutex_lock(&this->commandMapMutex);

    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) {
        if (this->commandMap[i]->key == notificationName || strcmp(this->commandMap[i]->key, notificationName) == 0) { // match
            if (out != NULL) // out param
                *out = this->commandMap[i]->factory;

            if (this->view->removeObserver(this->view, notificationName, self) == false) // remove observer
                fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeCommand] WARNING: Couldn't remove Observer for the notification '%s'; removing Command.\033[0m\n", notificationName);

            memset(&this->commandMap[i]->key, 0, sizeof(struct CommandMap)); // remove key only, factory is borrowed
            removed = true;
        } else {
            if (index != i) { // shift left
                *this->commandMap[index] = *this->commandMap[i]; // shift left first
                this->commandMap[i]->key = NULL; // remove
            }
            index++;
        }
    }
    mutex_unlock(&this->commandMapMutex);

    return removed;
}

size_t puremvc_controller_size() {
    return (sizeof(struct Controller) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

static struct IController *puremvc_controller_init(void *buffer, const char *key) {
    struct Controller *this = (struct Controller *) buffer;

    memset(this, 0, sizeof(struct Controller));

    this->base.initializeController = initializeController;
    this->base.executeCommand = executeCommand;
    this->base.registerCommand = registerCommand;
    this->base.hasCommand = hasCommand;
    this->base.removeCommand = removeCommand;

    this->multitonKey = key;
    mutex_init(&this->commandMapMutex);

    return (struct IController *) this;
}

static void dispatchOnce(void) {
    mutex_init(&controllerMapMutex);
}

struct IController *puremvc_controller_getInstance(struct ControllerMap **controllerMap, const char *key) {
    if (controllerMap == NULL && instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] FATAL: Missing ControllerMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    instanceMap = controllerMap;

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t i = 0;
    for (; instanceMap != NULL && instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) { // find controller
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
            mutex_unlock(&controllerMapMutex);
            return instanceMap[i]->controller;
        }
    }

    if (instanceMap == NULL || instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] FATAL: ControllerMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    if (instanceMap[i]->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Missing Controller storage; skipping registration.\033[0m\n");
        return NULL;
    }

    instanceMap[i]->key = key; // init
    puremvc_controller_init(instanceMap[i]->controller, key);

    mutex_unlock(&controllerMapMutex);
    return instanceMap[i]->controller;
}

bool puremvc_controller_removeController(const char *key, struct IController **out) {
    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] FATAL: Missing ControllerMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] FATAL: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t index = 0;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key != NULL; i++) {
        if (instanceMap[i]->key == key || strcmp(instanceMap[i]->key, key) == 0) {
            instanceMap[i]->key = NULL; // remove
            if (out != NULL)
                *out = instanceMap[i]->controller;
        } else {
            if (index != i) { // shift left
                *instanceMap[index] = *instanceMap[i];
                instanceMap[i]->key = NULL;
            }
            index++;
        }
    }

    if (index == 0) // all keys were removed; reset
        instanceMap = NULL;

    mutex_unlock(&controllerMapMutex);

    return true;
}

void puremvc_controller_reset() {
    mutex_lock(&controllerMapMutex);
    instanceMap = NULL;
    mutex_unlock(&controllerMapMutex);
}
