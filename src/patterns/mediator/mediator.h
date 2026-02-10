/**
* @file mediator.h
* @brief Mediator Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_mediator.h"
#include "../observer/notifier.h"

struct Mediator {
    struct IMediator base;

    const char *name;
    void *component;

    struct Notifier notifier;
};
