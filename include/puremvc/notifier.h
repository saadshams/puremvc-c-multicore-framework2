/**
* @file notifier.h
* @brief Notifier Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"
#include "i_notifier.h"

struct Facade; // forward declaration

struct Notifier {
    struct INotifier base;

    char key[NAME_SIZE];
};

struct INotifier *puremvc_notifier(struct Notifier *notifier);
