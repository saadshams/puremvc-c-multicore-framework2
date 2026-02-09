/**
* @file mediator.h
* @brief Mediator Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#pragma once

#include "puremvc/i_mediator.h"
#include "puremvc/notifier.h"

struct Mediator {
    struct IMediator base;

    struct Notifier notifier;

    void *component;

    size_t name_len;
    char name[];
};
