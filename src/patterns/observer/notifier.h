/**
* @file notifier.h
* @brief Notifier Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_notifier.h"

struct Facade; // forward declaration
struct IFacade;

struct Notifier {
    struct INotifier base;

    const char *key;
};
