/**
* @file Controller.c
* @internal
* @brief Controller Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#include "controller.h"

#include "puremvc/platform.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_command.h"

#include <stdio.h>
#include <string.h>

// instanceMap
static struct ControllerMap **controller_instanceMap = NULL;

// mutex for controllerMap
static Mutex controllerMapMutex;
static MutexOnce controllerMutexOnce = MUTEX_ONCE_INIT;

static void initializeController(struct IController *self, struct IView *view, struct CommandMap **commandMap) {
    struct Controller *this = (struct Controller *) self;

    if (view == NULL || commandMap == NULL) return;

    mutex_lock(&this->commandMapMutex);
    this->commandMap = commandMap;
    this->view = view;
    mutex_unlock(&this->commandMapMutex);
}

static bool registerCommand(struct IController *self, const char *notificationName, struct ICommand *(*factory)(void *buffer)) {
    struct Controller *this = (struct Controller *) self;

    if (notificationName == NULL || factory == NULL) return false;

    mutex_lock(&this->commandMapMutex);

    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[[PureMVC::Controller::registerCommand] Error: View unavailable to register Command '%s' — skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    if (this->commandMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] ERROR: Missing CommandMap field in ControllerMap; skipping registration.\033[0m\n");
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    size_t i = 0;
    for (; this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) { // existing
        if (this->commandMap[i]->key == notificationName || strcmp(this->commandMap[i]->key, notificationName) == 0) {
#ifndef NDEBUG
            printf("\033[0;36m[PureMVC::Controller::registerCommand] INFO: Command '%s' exists; overriding registration.\033[0m\n", notificationName);
#endif
            this->commandMap[i]->factory = factory; // registration (override)
            mutex_unlock(&this->commandMapMutex);
            return true;
        }
    }

    if (this->commandMap[i] == NULL) { // overflow (CommandMap)
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] Error: CommandMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    if (this->view->registerObserver(this->view, notificationName, (bool (*)(const void *context, const struct INotification *notification)) self->executeCommand, self)) {
        this->commandMap[i]->key = notificationName; // registration
        this->commandMap[i]->factory = factory;
        mutex_unlock(&this->commandMapMutex);
        return true;
    }

    mutex_unlock(&this->commandMapMutex);
    return false;
}

static bool executeCommand(const struct IController *self, struct INotification *notification) {
    struct Controller *this = (struct Controller *) self;
    bool success = false;
    const char *name = NULL;
    struct ICommand *(*factory)(void *) = NULL;

    if (notification == NULL) return false;

    name = notification->getName(notification);
    if (name == NULL) return false;

    if (mutex_lock_shared(&this->commandMapMutex) != 0) return false;

    if (this->commandMap == NULL) {
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    for (size_t i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) {
        if (this->commandMap[i]->key == name || strcmp(this->commandMap[i]->key, name) == 0) {
            factory = this->commandMap[i]->factory;
            break;
        }
    }

    mutex_unlock(&this->commandMapMutex);

    if (factory != NULL) {
        void *buffer = alloca(puremvc_simple_command_size());
        if (buffer == NULL) return false;

        const struct ICommand *command = factory(buffer);
        if (command == NULL) return false;

        struct INotifier *notifier = command->getNotifier(command);
        if (notifier == NULL) return false;
        notifier->initializeNotifier(notifier, this->multitonKey);
        success = command->execute(command, notification);
    }

    return success;
}

static bool hasCommand(const struct IController *self, const char *notificationName) {
    struct Controller *this = (struct Controller *) self;
    bool exists = false;

    if (notificationName == NULL) return false;

    if (mutex_lock_shared(&this->commandMapMutex) != 0) return false;

    if (this->commandMap == NULL) {
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    for (size_t i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) {
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

    if (notificationName == NULL) return false;

    mutex_lock(&this->commandMapMutex);
    if (this->commandMap == NULL) {
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    for (size_t index = 0, i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key != NULL; i++) { // One-pass Filter & Shift (O(n) complexity)
        if (this->commandMap[i]->key == notificationName || strcmp(this->commandMap[i]->key, notificationName) == 0) { // match
            if (out != NULL) // out param
                *out = this->commandMap[i]->factory;

            if (this->view->removeObserver(this->view, notificationName, self) == false) // remove observer
                fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeCommand] Error: Couldn't remove Observer for the notification '%s'; removing Command.\033[0m\n", notificationName);

            this->commandMap[i]->key = NULL; // remove key only, factory is borrowed
            removed = true;
        } else {
            if (index != i) { // shift left (Gap-free array)
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

    if (buffer == NULL) return NULL;

    memset(this, 0, sizeof(struct Controller));

    this->base.initializeController = initializeController;
    this->base.executeCommand = executeCommand;
    this->base.registerCommand = registerCommand;
    this->base.hasCommand = hasCommand;
    this->base.removeCommand = removeCommand;

    this->multitonKey = key;

    if (mutex_init(&this->commandMapMutex) != 0) {
#ifndef NDEBUG
        fprintf(stderr, "\033[0;31m[PureMVC] ERROR: Mutex initialization failed for key '%s'.\033[0m\n", key);
#endif
        return NULL;
    }
    return (struct IController *) this;
}

static void dispatchOnce(void) {
    if (mutex_init(&controllerMapMutex) != 0) {
        fprintf(stderr, "\033[PureMVC::Controller::getInstance] ERROR: Mutex Init Failed!\033[0m\n");
    }
}

struct IController *puremvc_controller_getInstance(struct ControllerMap **controllerMap, const char *key) {
    if (controllerMap == NULL && controller_instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: Missing ControllerMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    controller_instanceMap = controllerMap;
    mutex_once(&controllerMutexOnce, dispatchOnce);

    mutex_lock(&controllerMapMutex);

    size_t i = 0;
    for (; controller_instanceMap != NULL && controller_instanceMap[i] != NULL && controller_instanceMap[i]->key != NULL; i++) { // find controller
        if (controller_instanceMap[i]->key == key || strcmp(controller_instanceMap[i]->key, key) == 0) {
            mutex_unlock(&controllerMapMutex);
            return controller_instanceMap[i]->controller;
        }
    }

    if (controller_instanceMap == NULL || controller_instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: ControllerMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    if (controller_instanceMap[i]->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] ERROR: Missing Controller storage; skipping registration.\033[0m\n");
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    controller_instanceMap[i]->key = key; // init
    puremvc_controller_init(controller_instanceMap[i]->controller, key);

    mutex_unlock(&controllerMapMutex);
    return controller_instanceMap[i]->controller;
}

bool puremvc_controller_removeController(const char *key, struct IController **out) {
    bool removed = false;

    if (controller_instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] ERROR: Missing ControllerMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] ERROR: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t index = 0;
    for (size_t i = 0; controller_instanceMap[i] != NULL && controller_instanceMap[i]->key != NULL; i++) {
        if (controller_instanceMap[i]->key == key || strcmp(controller_instanceMap[i]->key, key) == 0) {
            controller_instanceMap[i]->key = NULL; // remove
            if (out != NULL)
                *out = controller_instanceMap[i]->controller;
            removed = true;
        } else {
            if (index != i) { // shift left
                *controller_instanceMap[index] = *controller_instanceMap[i];
                controller_instanceMap[i]->key = NULL;
            }
            index++;
        }
    }

    mutex_unlock(&controllerMapMutex);

    return removed;
}

void puremvc_controller_reset() {
    mutex_lock(&controllerMapMutex);
    controller_instanceMap = NULL;
    mutex_unlock(&controllerMapMutex);
}
