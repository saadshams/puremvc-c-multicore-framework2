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

}

static void onRemove(struct Proxy *self) {

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

    snprintf(proxy.name, NAME_SIZE, "%s", name ? name : PROXY_NAME);
    return proxy;
}
