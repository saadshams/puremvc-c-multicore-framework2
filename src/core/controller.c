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
#include "puremvc/simple_command.h"
#include "puremvc/view.h"

#include <stdio.h>
#include <string.h>

// controllerMap
static struct ControllerMap **s_controllerMap = NULL;

// mutex for controllerMap
static Mutex controllerMapMutex;
static MutexOnce controllerMutexOnce = MUTEX_ONCE_INIT;

static void initializeController(struct IController *self) {
    struct Controller *this = (struct Controller *) self;
    if (this->view != NULL) return;
    this->view = puremvc_view_getInstance(NULL, this->multitonKey);
}

static bool registerCommand(struct IController *self, const char *notificationName, struct ICommand *(*factory)(struct ICommand *)) {
    struct Controller *this = (struct Controller *) self;

    if (this->view == NULL) {
        fprintf(stderr, "\033[0;31m[[PureMVC::Controller::registerCommand] Error: View unavailable to register Command '%s' — skipping registration.\033[0m\n", notificationName);
        return false;
    }

    if (strlen(notificationName) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "\033[0;31m[[PureMVC::Controller::registerCommand] Error: Key '%s' too long (max %d) — skipping registration.\033[0m\n", notificationName, KEY_SIZE);
        return false;
    }

    if (this->commandMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::registerCommand] FATAL: Missing CommandMap field in ControllerMap; skipping registration.\033[0m\n");
        return false;
    }

    mutex_lock(&this->commandMapMutex);

    size_t i = 0;
    for (; this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) { // existing
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) {
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
        snprintf(this->commandMap[i]->key, KEY_SIZE, "%s", notificationName); // registration
        this->commandMap[i]->factory = factory;
        mutex_unlock(&this->commandMapMutex);
        return true;
    }

    return false;
}

static void executeCommand(const struct IController *self, struct INotification *notification) {
    struct Controller *this = (struct Controller *) self;
    mutex_lock_shared(&this->commandMapMutex);

    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notification->getName(notification)) == 0) {
            struct ICommand *(*factory)(struct ICommand *) = this->commandMap[i]->factory;
            const struct ICommand *command = factory((struct ICommand *) &(struct SimpleCommand){});
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
    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) {
            exists = true;
            break;
        }
    }
    mutex_unlock(&this->commandMapMutex);
    return exists;
}

static bool removeCommand(struct IController *self, const char *notificationName, struct ICommand *(**factory)(struct ICommand *)) {
    struct Controller *this = (struct Controller *) self;
    bool removed = false;

    mutex_lock(&this->commandMapMutex);

    size_t index = 0; // One-pass removal (Filter pattern)
    for (size_t i = 0; this->commandMap != NULL && this->commandMap[i] != NULL && this->commandMap[i]->key[0] != '\0'; i++) {
        if (strcmp(this->commandMap[i]->key, notificationName) == 0) { // match
            if (factory != NULL) // out param
                *factory = this->commandMap[i]->factory;

            if (this->view->removeObserver(this->view, notificationName, self) == false) // remove observer
                fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeCommand] WARNING: Couldn't remove Observer for the notification '%s'; removing Command.\033[0m\n", notificationName);

            memset(&this->commandMap[i]->key, 0, sizeof(struct CommandMap)); // remove
            removed = true;
        } else {
            if (index != i) { // shift left
                *this->commandMap[index] = *this->commandMap[i]; // shift left first
                memset(&this->commandMap[i]->key, 0, KEY_SIZE); // remove
            }
            index++;
        }
    }
    mutex_unlock(&this->commandMapMutex);

    return removed;
}

static void puremvc_controller_init(struct IController *controller, const char *key) {
    struct Controller *this = (struct Controller *) controller;

    controller->initializeController = initializeController;
    controller->executeCommand = executeCommand;
    controller->registerCommand = registerCommand;
    controller->hasCommand = hasCommand;
    controller->removeCommand = removeCommand;

    snprintf(this->multitonKey, KEY_SIZE, "%s", key);
    mutex_init(&this->commandMapMutex);
}

static void dispatchOnce(void) {
    mutex_init(&controllerMapMutex);
}

struct IController *puremvc_controller_getInstance(struct ControllerMap **controllerMap, const char *key) {
    if (controllerMap == NULL && s_controllerMap == NULL) {
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

    if (s_controllerMap == NULL)
        s_controllerMap = controllerMap;

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t i = 0;
    for (; s_controllerMap[i] != NULL && s_controllerMap[i]->key[0] != '\0'; i++) { // find controller
        if (strncmp(s_controllerMap[i]->key, key, KEY_SIZE) == 0) {
            mutex_unlock(&controllerMapMutex);
            return s_controllerMap[i]->controller;
        }
    }

    if (s_controllerMap[i] == NULL) { // overflow
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::getInstance] FATAL: ControllerMap storage overflow for the key '%s'; increase slots - skipping registration.\033[0m\n", key);
        mutex_unlock(&controllerMapMutex);
        return NULL;
    }

    if (s_controllerMap[i]->controller == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::View::getInstance] FATAL: Missing Controller storage; skipping registration.\033[0m\n");
        return NULL;
    }

    snprintf(s_controllerMap[i]->key, KEY_SIZE, "%s", key); // init
    puremvc_controller_init(s_controllerMap[i]->controller, key);
    s_controllerMap[i]->controller->initializeController(s_controllerMap[i]->controller);

    mutex_unlock(&controllerMapMutex);
    return s_controllerMap[i]->controller;
}

bool puremvc_controller_removeController(const char *key, struct IController **controller) {
    if (s_controllerMap == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] FATAL: Missing ControllerMap storage; skipping removal.\033[0m\n");
        return false;
    }

    if (key == NULL) {
        fprintf(stderr, "\033[0;31m[PureMVC::Controller::removeController] FATAL: Key is NULL; skipping removal.\033[0m\n");
        return false;
    }

    if (strlen(key) >= KEY_SIZE) { // Key truncation collision
        fprintf(stderr, "[PureMVC::Controller::removeController] Error: Key '%s' too long (max %d) — skipping removal.\n", key, KEY_SIZE);
        return false;
    }

    mutex_once(&controllerMutexOnce, dispatchOnce);
    mutex_lock(&controllerMapMutex);

    size_t index = 0;
    for (size_t i = 0; s_controllerMap[i] != NULL && s_controllerMap[i]->key[0] != '\0'; i++) {
        if (strncmp(s_controllerMap[i]->key, key, KEY_SIZE) == 0) {
            memset(s_controllerMap[i]->key, 0, KEY_SIZE); // remove
            if (controller != NULL)
                *controller = s_controllerMap[i]->controller;
        } else {
            if (index != i) { // shift left
                *s_controllerMap[index] = *s_controllerMap[i];
                memset(s_controllerMap[i]->key, 0, KEY_SIZE);
            }
            index++;
        }
    }

    if (index == 0) // all keys were removed; reset
        s_controllerMap = NULL;

    mutex_unlock(&controllerMapMutex);

    return true;
}
