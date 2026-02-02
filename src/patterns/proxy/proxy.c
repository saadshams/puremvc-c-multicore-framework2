/**
* @file proxy.c
* @internal
* @brief Proxy Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/proxy.h"

#include <stdio.h>

static const char *getName(const struct IProxy *self) {
    const struct Proxy *this = (struct Proxy *) self;
    return this->name;
}

static void *getData(const struct IProxy *self) {
    const struct Proxy *this = (struct Proxy *) self;
    return this->data;
}

static void setData(struct IProxy *self, void *data) {
    struct Proxy *this = (struct Proxy *) self;
    this->data = data;
}

static void onRegister(struct IProxy *self) {
    (void)self;
}

static void onRemove(struct IProxy *self) {
    (void)self;
}

struct Proxy puremvc_proxy(const char *name, void *data) {
    struct Proxy proxy = {
        .base = (struct IProxy) {
            .getName = getName,
            .getData = getData,
            .setData = setData,
            .onRegister = onRegister,
            .onRemove = onRemove,
        },
        .data = data,
        .notifier = puremvc_notifier(),
    };

    int len = snprintf(proxy.name, NAME_SIZE, "%s", name ? name : PROXY_NAME);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Proxy] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : PROXY_NAME, len, NAME_SIZE);

    return proxy;
}
