/**
* @file proxy.h
* @brief Proxy Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#pragma once

#include "puremvc/i_proxy.h"
#include "../observer//notifier.h"

struct Proxy {
    struct IProxy base;

    struct Notifier notifier;

    const char *name;
    void *data;
};
