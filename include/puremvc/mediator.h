/**
* @file mediator.h
* @brief Mediator Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "constants.h"
#include "i_mediator.h"
#include "notifier.h"

#define MEDIATOR_NAME "mediator"

struct Mediator {
    struct IMediator base;

    char name[NAME_SIZE];
    void *component;

    struct Notifier notifier;
};

struct Mediator puremvc_mediator(const char *name, void *component);
