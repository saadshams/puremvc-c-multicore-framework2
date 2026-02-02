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

// void puremvc_proxy_teardown(struct Proxy *self) {
//     if (!self) return;
//
//      Call notifier specific cleanup if necessary
//      puremvc_notifier_teardown(&proxy->notifier);
//     // 1. Trigger the lifecycle hook via the interface
//     if (self->base.onRemove) {
//         self->base.onRemove(&self->base);
//     }
//
//     // 2. Clear out data to prevent accidental reuse
//     self->data = NULL;
//
//     // 3. Clear the name buffer
//     memset(self->name, 0, NAME_SIZE);
//
//     // 4. Optionally nullify the function pointers in the base
//     // This prevents calling methods on a "destroyed" proxy
//     memset(&self->base, 0, sizeof(struct IProxy));
// }