/**
* @file proxy.h
* @brief Proxy Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#pragma once

#include "puremvc/platform.h"
#include "puremvc/i_proxy.h"
#include "../observer/notifier.h"

struct Proxy {
    // alignas(ALIGNMENT)
    struct IProxy base; // Ensures the struct starts at an ALIGNMENT boundary (e.g., pointer size)

    char name[KEY_SIZE];
    void *data;

    struct Notifier notifier;
};
