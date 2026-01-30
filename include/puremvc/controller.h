/**
* @file Controller.h
* @internal
* @brief Controller Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"
#include "mutex.h"
#include "view.h"
#include "simple_command.h"
#include "notification.h"

struct Controller {
    char multitonKey[KEY_SIZE];
    struct View *view;

    Mutex commandMapMutex;
    struct CommandMap {
        char key[KEY_SIZE];
        struct SimpleCommand (*factory)();
    } commandMap[COMMAND_MAP_SIZE];

    void (*initializeController)(struct Controller *self);
    
    void (*registerCommand)(struct Controller *self, const char *notificationName, struct SimpleCommand(*factory)());

    void (*executeCommand)(struct Controller *self, struct Notification notification);

    bool (*hasCommand)(struct Controller *self, const char *notificationName);

    void (*removeCommand)(struct Controller *self, const char *notificationName);
};

struct Controller puremvc_controller(const char *key);

struct Controller *puremvc_controller_getInstance(const char *key, struct Controller(*factory)(const char *key));

void puremvc_controller_removeController(const char *key);
