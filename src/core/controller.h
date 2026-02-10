/**
* @file Controller.h
* @internal
* @brief Controller Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_mutex.h"
#include "puremvc/i_controller.h"
#include "puremvc/i_view.h"

struct Controller {
    struct IController base;

    const char *multitonKey;
    struct IView *view;
    struct CommandMap **commandMap;

    Mutex commandMapMutex;
};
