/**
* @file proxy.c
* @internal
* @brief Proxy Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include <stdio.h>

#include "puremvc/proxy.h"

static const char *getName(const struct Proxy *self) {
    return self->name;
}

static void *getData(const struct Proxy *self) {
    return self->data;
}

static void setData(struct Proxy *self, void *data) {
    self->data = data;
}

static void onRegister(struct Proxy *self) {
    (void)self;
}

static void onRemove(struct Proxy *self) {
    (void)self;
}

struct Proxy puremvc_proxy(const char *name, void *data) {
    struct Proxy proxy = {
        .data = data,
        .notifier = puremvc_notifier(),
        .getName = getName,
        .getData = getData,
        .setData = setData,
        .onRegister = onRegister,
        .onRemove = onRemove
    };

    int len = snprintf(proxy.name, NAME_SIZE, "%s", name ? name : PROXY_NAME);
    if (len >= NAME_SIZE)
        printf("[PureMVC::Proxy] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : PROXY_NAME, len, NAME_SIZE);

    return proxy;
}
