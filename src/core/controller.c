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
static struct ControllerMap **instanceMap = NULL;

// mutex for controllerMap
static Mutex instanceMapMutex;
static MutexOnce mutexOnce = MUTEX_ONCE_INIT;

static void initializeController(struct IController *self, struct IView *view, struct CommandMap **commandMap) {
    struct Controller *this = (struct Controller *) self;
    this->commandMap = commandMap;
    this->view = view;
}

static bool registerCommand(struct IController *self, const char *notificationName, struct ICommand *(*factory)(void *buffer)) {
    if (notificationName == NULL || factory == NULL) return false;
    struct Controller *this = (struct Controller *) self;
    bool registered = false;

    mutex_lock(&this->commandMapMutex);

    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] Error: View unavailable to register Command '%s' — skipping registration.\033[0m\n", notificationName);
        goto finally;
    }

    if (this->commandMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] ERROR: Missing CommandMap field in ControllerMap; skipping registration.\033[0m\n");
        goto finally;
    }

    size_t i = 0;
    for (; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) { // existing
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) {
#ifndef NDEBUG
            printf("\033[0;36m[PureMVC::Controller::registerCommand] INFO: Command '%s' exists; overriding registration.\033[0m\n", notificationName);
#endif
            this->commandMap[i]->factory = factory; // registration (override)
            registered = true;
            goto finally;
        }
    }

    if (this->commandMap[i] == NULL) { // overflow (CommandMap)
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] Error: CommandMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        goto finally;;
    }

    if (this->view->registerObserver(this->view, notificationName, (void (*)(const void *context, const struct INotification *notification)) self->executeCommand, self)) {
        int len = snprintf(this->commandMap[i]->key, KEY_SIZE, "%s", notificationName); // registration
        if (len < 0 || len >= KEY_SIZE) { // todo reset command
            fprintf(stderr, "\033[0;31m[PureMVC::Model::registerProxy] Error: ProxyMap key truncated: '%s' (max %d chars).\033[0m\n", notificationName, KEY_SIZE);
            memset(this->commandMap[i]->key, 0, KEY_SIZE);
            goto finally;;
        }

        this->commandMap[i]->factory = factory;
        registered = true;
    }

finally:
    mutex_unlock(&this->commandMapMutex);
    return registered;
}

static void executeCommand(const struct IController *self, struct INotification *notification) {
    if (notification == NULL) return;
    struct Controller *this = (struct Controller *) self;
    struct ICommand *(*factory)(void *) = NULL;

    mutex_lock_shared(&this->commandMapMutex);
    for (size_t i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notification->getName(notification)) == 0) {
            factory = this->commandMap[i]->factory;
            break;
        }
    }
    mutex_unlock(&this->commandMapMutex);

    if (factory == NULL) return;

    const struct ICommand *command = factory(alloca(puremvc_simple_command_size()));
    struct INotifier *notifier = command->getNotifier(command);
    notifier->initializeNotifier(notifier, this->multitonKey);
    command->execute(command, notification);
}

static bool hasCommand(const struct IController *self, const char *notificationName) {
    if (notificationName == NULL) return false;
    struct Controller *this = (struct Controller *) self;
    bool exists = false;

    mutex_lock_shared(&this->commandMapMutex);
    for (size_t i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->commandMapMutex);

    return exists;
}

static bool removeCommand(struct IController *self, const char *notificationName, struct ICommand *(**out)(void *buffer)) {
    if (notificationName == NULL) return false;
    struct Controller *this = (struct Controller *) self;
    bool removed = false;

    mutex_lock(&this->commandMapMutex);
    size_t index = 0;
    for (size_t i = 0; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) { // One-pass Filter & Shift (O(n) complexity)
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) { // match
            if (out != NULL) *out = this->commandMap[i]->factory; // out param
            this->view->removeObserver(this->view, notificationName, self);
            memset(&this->commandMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i)
                *this->commandMap[index] = *this->commandMap[i]; // shift left (Gap-free array)

            index++;
        }
    }
    memset(&this->commandMap[index]->key, 0, KEY_SIZE); // reset tail slot
    mutex_unlock(&this->commandMapMutex);

    return removed;
}

size_t puremvc_controller_size(void) {
    return (sizeof(struct Controller) + (sizeof(void *) -1u)) & ~(sizeof(void *) - 1u);
}

static struct IController *puremvc_controller_init(void *buffer, const char *key) {
    if (buffer == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::init] Error: Buffer is NULL for controller '%s' - skipping initialization.\033[0m\n", key);
        return NULL;
    }

    struct Controller *this = buffer;
    memset(this, 0, sizeof(struct Controller));

    this->super.initializeController = initializeController;
    this->super.executeCommand = executeCommand;
    this->super.registerCommand = registerCommand;
    this->super.hasCommand = hasCommand;
    this->super.removeCommand = removeCommand;

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
    if (mutex_init(&instanceMapMutex) != 0) {
        fprintf(stderr, "\033[PureMVC::Controller::getInstance] ERROR: Mutex Init Failed!\033[0m\n");
    }
}

struct IController *puremvc_controller_getInstance(struct ControllerMap **controllerMap, const char *key) {
    if (controllerMap == NULL || key == NULL) return NULL;
    struct IController *controller = NULL;

    instanceMap = controllerMap;

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t i = 0;
    for (; instanceMap != NULL && instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) { // find controller
        if (strcmp(instanceMap[i]->key, key) == 0) { // match
            controller = instanceMap[i]->controller;
            goto finally;
        }
    }

    if (instanceMap == NULL || instanceMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: ControllerMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        goto finally;
    }

    if (instanceMap[i]->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] ERROR: Missing Controller storage; skipping registration.\033[0m\n");
        goto finally;
    }

    int len = snprintf(instanceMap[i]->key, KEY_SIZE, "%s", key); // registration
    if (len < 0 || len >= KEY_SIZE) { // todo reset controller
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] Error: ControllerMap key truncated: '%s' (max %d chars).\033[0m\n", key, KEY_SIZE);
        memset(instanceMap[i]->key, 0, KEY_SIZE);
        goto finally;
    }

    controller = puremvc_controller_init(instanceMap[i]->controller, key); // init

finally:
    mutex_unlock(&instanceMapMutex);
    return controller;
}

bool puremvc_controller_removeController(const char *key, struct IController **out) {
    if (key == NULL) return false;
    bool removed = false;

    if (instanceMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] ERROR: Missing ControllerMap storage; skipping removal.\033[0m\n");
        return false;
    }

    mutex_once(&mutexOnce, dispatchOnce);
    mutex_lock(&instanceMapMutex);

    size_t index = 0;
    for (size_t i = 0; instanceMap[i] != NULL && instanceMap[i]->key[0] != '\0'; i++) {
        if (strcmp(instanceMap[i]->key, key) == 0) {
            if (out != NULL) *out = instanceMap[i]->controller;
            memset(instanceMap[i]->key, 0, KEY_SIZE); // remove
            removed = true;
        } else {
            if (index != i)
                *instanceMap[index] = *instanceMap[i]; // shift left (Gap-free array)

            index++;
        }
    }
    memset(instanceMap[index]->key, 0, KEY_SIZE); // reset tail slot

    if (index == 0) instanceMap = NULL;

    mutex_unlock(&instanceMapMutex);
    return removed;
}
