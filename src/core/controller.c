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

// instanceMap (global)
static struct ControllerMap **instanceMap = NULL;

// mutex for controllerMap (global)
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
    for (; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) { // existing
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
        int len = snprintf(this->commandMap[i]->key, KEY_SIZE, "%s", notificationName); // registration
        if (len < 0 || len >= KEY_SIZE) { // todo reset command
            fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ProxyMap key truncated: '%s' (max %d chars).\033[0m\n", notificationName, KEY_SIZE);
            memset(this->commandMap[i]->key, 0, KEY_SIZE);
            mutex_unlock(&this->commandMapMutex);
            return false;
        }

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

    if (notification == NULL) return false;

    const char *name = NULL;
    name = notification->getName(notification);
    if (name == NULL) return false;

    if (mutex_lock_shared(&this->commandMapMutex) != 0) return false;

    if (this->commandMap == NULL) {
        mutex_unlock(&this->commandMapMutex);
        return false;
    }

    struct ICommand *(*factory)(void *) = NULL;
    for (size_t i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
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

    for (size_t i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
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

    for (size_t index = 0, i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) { // One-pass Filter & Shift (O(n) complexity)
        if (this->commandMap[i]->key == notificationName || strcmp(this->commandMap[i]->key, notificationName) == 0) { // match
            if (out != NULL) // out param
                *out = this->commandMap[i]->factory;

            if (this->view->removeObserver(this->view, notificationName, self) == false) // remove observer
                fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeCommand] Error: Couldn't remove Observer for the notification '%s'; removing Command.\033[0m\n", notificationName);

            memset(&this->commandMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i) { // shift left (Gap-free array)
                *this->commandMap[index] = *this->commandMap[i]; // shift left first
                memset(&this->commandMap[i]->key, 0, KEY_SIZE); // remove
            }
            index++;
        }
    }
    mutex_unlock(&this->commandMapMutex);

    return removed;
}

size_t puremvc_controller_size() {
    return (sizeof(struct Controller) + (sizeof(void *) -1u)) & ~(sizeof(void *) - 1u);
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

    int len = snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    if (len < 0 || len >= KEY_SIZE) {
        memset(this, 0, sizeof(struct Controller));
        fprintf(stderr, "\033[0;31m[PureMVC::View::init] Error: Controller multitonKey truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        return NULL;
    }

    if (mutex_init(&this->commandMapMutex) != 0) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::init] ERROR: Mutex initialization failed for key '%s'.\033[0m\n", key);
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
    if (controllerMap == NULL && instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: Missing ControllerMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    instanceMap = controllerMap;
    mutex_once(&controllerMutexOnce, dispatchOnce);

    mutex_lock(&controllerMapMutex);

    size_t i = 0;
    for (; instanceMap != NULL && instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find controller
        if (strcmp(instanceMap[i]->key, key) == 0) {
            mutex_unlock(&controllerMapMutex);
            return instanceMap[i]->controller;
        }
    }

    if (instanceMap == NULL || instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: ControllerMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    if (instanceMap[i]->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] ERROR: Missing Controller storage; skipping registration.\033[0m\n");
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    int len = snprintf(instanceMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len < 0 || len >= KEY_SIZE) { // todo reset controller
        fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ControllerMap key truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        memset(instanceMap[i]->key, 0, KEY_SIZE);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    puremvc_controller_init(instanceMap[i]->controller, key); // init
    printf("Controller Storing: requested key='%s', map key: '%s', pointer %p\n", key, instanceMap[i]->key, instanceMap[i]->controller);
    fflush(stdout);

    mutex_unlock(&controllerMapMutex);
    return instanceMap[i]->controller;
}

bool puremvc_controller_removeController(const char *key, struct IController **out) {
    bool removed = false;

    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] ERROR: Missing ControllerMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] ERROR: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t i = 0, index = 0;
    for (; instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) {
        if (strcmp(instanceMap[i]->key, key) == 0) {
            printf("Controller Removing: requested key='%s', map key: '%s', pointer %p\n", key, instanceMap[i]->key, instanceMap[i]->controller);
            fflush(stdout);
            memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            if (out != NULL)
                *out = instanceMap[i]->controller;
            removed = true;
        } else {
            if (index != i) { // shift left (Gap-free array)
                *instanceMap[index] = *instanceMap[i]; // shift left first
                memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            }
            index++;
        }
    }

    if (index == 0) instanceMap = NULL; // avoid dangling global stack pointer after removal of last entry

    mutex_unlock(&controllerMapMutex);

    return removed;
}
