/**
* @file proxy.h
* @brief Proxy Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#pragma once

#include "constants.h"
#include "notifier.h"

#define PROXY_NAME "Proxy"

struct Proxy {
    char name[MAX_NAME_LEN];
    void* data;

    struct Notifier notifier;

    const char* (*getName)(const struct Proxy *self);
    void* (*getData)(const struct Proxy *self);
    void (*setData)(struct Proxy* self, void *data);

    void (*onRegister)(struct Proxy *self);
    void (*onRemove)(struct Proxy *self);
};

struct Proxy puremvc_proxy(const char *name, void *data);
