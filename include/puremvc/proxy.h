/**
* @file proxy.h
* @brief Proxy Header
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/

#pragma once

#include "constants.h"
#include "i_proxy.h"
#include "notifier.h"

#define PROXY_NAME "Proxy"

struct Proxy {
    struct IProxy base;

    char name[NAME_SIZE];
    void *data;

    struct Notifier notifier;
};

struct IProxy *puremvc_proxy_init(struct Proxy *proxy, const char *name, void *data);

void puremvc_proxy_deinit(struct Proxy *proxy);
