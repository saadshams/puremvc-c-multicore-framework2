/**
* @file Controller.h
* @internal
* @brief Controller Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "mutex.h"
#include "constants.h"
#include "i_controller.h"
#include "simple_command.h"
#include "view.h"

struct Controller {
    struct IController base;

    char multitonKey[KEY_SIZE];
    struct IView *view;

    Mutex commandMapMutex;
    struct CommandMap {
        char key[KEY_SIZE];
        struct SimpleCommand (*factory)();
    } commandMap[COMMAND_MAP_SIZE];
};

struct Controller puremvc_controller(const char *key);

struct IController *puremvc_controller_getInstance(const char *key, struct Controller(*factory)(const char *key));

void puremvc_controller_removeController(const char *key);
