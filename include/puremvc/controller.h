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
#include "i_view.h"

struct Controller {
    struct IController base;

    char multitonKey[KEY_SIZE];
    struct IView *view;

    Mutex commandMapMutex;
    struct CommandMap {
        char key[KEY_SIZE];
        struct ICommand *(*factory)(struct ICommand *);
    } **commandMap;
};

struct ControllerMap {
    char key[KEY_SIZE];
    struct Controller controller;
};

struct IController *puremvc_controller_getInstance(struct ControllerMap **controllerMap, const char *key);

void puremvc_controller_removeController(struct ControllerMap **controllerMap, const char *key);
