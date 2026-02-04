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

// mutex for controllerMap
static Mutex controllerMapMutex;
static MutexOnce controllerMutexOnce = MUTEX_ONCE_INIT;

static void initializeController(const struct IController *self) {
    struct Controller *this = (struct Controller *) self;
    if (this->view != NULL) return;
    this->view = puremvc_view_getInstance(this->multitonKey);
}

static void registerCommand(struct IController *self, const char *notificationName, struct SimpleCommand(*factory)()) {
    struct Controller *this = (struct Controller *) self;

    if (strlen(notificationName) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Controller::registerCommand] Error: Key '%s' too long (max %d) — skipping registration.\n", notificationName, KEY_SIZE);
        return;
    }

    if (this->commandMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] FATAL: Missing CommandMap field in ControllerMap; skipping registration.\033[0m\n");
        return;
    }

    mutex_lock(&this->commandMapMutex);

    size_t i = 0;
    for (; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) { // existing
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) {
            fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] Warning: Command '%s' exists; overridden registration\033[0m.\n", proxy.base.getName(&proxy.base));

            this->commandMap[i]->factory = factory; // registration
            mutex_unlock(&this->commandMapMutex);
            return;
        }
    }

    if (this->commandMap[i] == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] Error: CommandMap storage overflow for notification '%s'; increase slots - skipping registration.\033[0m\n", notificationName);
        mutex_unlock(&this->commandMapMutex);
        return;
    }

    const struct Observer observer = puremvc_observer((void (*)(const void *, struct INotification *)) self->executeCommand, self);
    this->view->registerObserver(this->view, notificationName, observer);

    snprintf(this->commandMap[i]->key, KEY_SIZE, "%s", notificationName); // registration
    this->commandMap[i]->factory = factory;
    mutex_unlock(&this->commandMapMutex);
}

static void executeCommand(const struct IController *self, struct INotification *notification) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock_shared(&this->commandMapMutex);

    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notification->getName(notification)) == 0) {
            struct SimpleCommand (*factory)() = this->commandMap[i]->factory;
            struct SimpleCommand command = factory();
            command.notifier.base.initializeNotifier(&command.notifier.base, this->multitonKey);
            command.base.execute(&command.base, notification);
            puremvc_simple_command_deinit(&command);
            break;
        }
    }

    mutex_unlock(&this->commandMapMutex);
}

static bool hasCommand(const struct IController *self, const char *notificationName) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock_shared(&this->commandMapMutex);
    bool exists = false;
    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) {
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
    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) { // match
            this->view->removeObserver(this->view, notificationName, self);
            memset(&this->commandMap[i], 0, sizeof(struct CommandMap));
        } else {
            if (index != i) { // shift left
                this->view->removeObserver(this->view, notificationName, self); // remove observer before the shift

                snprintf(this->commandMap[index]->key, KEY_SIZE, "%s", this->commandMap[i]->key);
                this->commandMap[index]->factory = this->commandMap[i]->factory;;

                const struct Observer observer = puremvc_observer((void (*)(const void *, struct INotification *)) executeCommand, self);
                this->view->registerObserver(this->view, notificationName, observer); // register after the shift

                memset(&this->commandMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }
    mutex_unlock(&this->commandMapMutex);
}

static void init(struct Controller *controller, const char *key) {
    controller->base = (struct IController) {
        .initializeController = initializeController,
        .executeCommand = executeCommand,
        .registerCommand = registerCommand,
        .hasCommand = hasCommand,
        .removeCommand = removeCommand
    };

    snprintf(controller->multitonKey, KEY_SIZE, "%s", key);
    mutex_init(&controller->commandMapMutex);
}

static void deinit(struct Controller *controller) {
    memset(controller->multitonKey, 0, KEY_SIZE);
    controller->base = (struct IController){0};

    mutex_destroy(&controller->commandMapMutex);

    for (size_t j = 0; controller->commandMap != NULL && controller->commandMap[j] != NULL; j++) { // clear commandMap
        memset(controller->commandMap[j]->key, 0, KEY_SIZE);
        controller->commandMap[j]->factory = NULL;
    }
}

static void dispatchOnce(void) {
    mutex_init(&controllerMapMutex);
}

struct IController *puremvc_controller_getInstance(struct ControllerMap **controllerMap, const char *key) {
    if (controllerMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] FATAL: Missing ControllerMap storage; skipping registration.\033[0m\n");
        return NULL;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return NULL;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Controller::getInstance] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return NULL;
    }

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t i = 0;
    for (; controllerMap[i] != NULL && controllerMap[i]->key[0] != '\0'; i++) { // find controller
        if (strncmp(controllerMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&controllerMapMutex);
            return &controllerMap[i]->controller.base;
        }
    }

    if (controllerMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] FATAL: ControllerMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    snprintf(controllerMap[i]->key, KEY_SIZE, "%s", key); // init
    init(&controllerMap[i]->controller, key);
    controllerMap[i]->controller.base.initializeController(&controllerMap[i]->controller.base);

    mutex_unlock(&controllerMapMutex);
    return &controllerMap[i]->controller.base;
}

void puremvc_controller_removeController(struct ControllerMap **controllerMap, const char *key) {
    if (controllerMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] FATAL: Missing ControllerMap storage; skipping registration.\033[0m\n");
        return;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] FATAL: Key is NULL; skipping registration.\033[0m\n");
        return;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Controller::removeController] Error: Key '%s' too long (max %d) — skipping registration.\n", key, KEY_SIZE);
        return;
    }

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t index = 0;
    for (size_t i = 0; controllerMap[i] != NULL && controllerMap[i]->key[0] != '\0'; i++) {
        if (strncmp(controllerMap[i]->key, key, KEY_SIZE) == 0) {
            memset(controllerMap[i]->key, 0, KEY_SIZE); // clear controller
        } else {
            if (index != i) {
                snprintf(controllerMap[index]->key, KEY_SIZE, "%s", controllerMap[i]->key); // copy model (destination)
                controllerMap[index]->controller = controllerMap[i]->controller;

                memset(controllerMap[i]->key, 0, KEY_SIZE); // clear model (source)
                deinit(&controllerMap[i]->controller);
            }
            index++;
        }
    }
    mutex_unlock(&controllerMapMutex);
}
