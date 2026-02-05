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

#define MEDIATOR_NAME "mediator"

struct Mediator {
    struct IMediator base;

    char name[NAME_SIZE];
    void *component;
};

struct IMediator *puremvc_mediator(struct Mediator *mediator, const char *name, void *component);

void puremvc_mediator_deinit(struct Mediator *mediator);
