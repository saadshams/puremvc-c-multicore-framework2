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
    struct Proxy proxy = {0};

    snprintf(proxy.name, MAX_NAME_LEN, "%s", name ? name : PROXY_NAME);
    proxy.data = data;

    proxy.notifier = puremvc_notifier();

    proxy.getName = getName;
    proxy.getData = getData;
    proxy.setData = setData;
    proxy.onRegister = onRegister;
    proxy.onRemove = onRemove;

    return proxy;
}
