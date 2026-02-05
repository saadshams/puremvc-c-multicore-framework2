/**
* @file proxy.c
* @internal
* @brief Proxy Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "puremvc/proxy.h"
#include "puremvc/notifier.h"

#include <stdio.h>
#include <string.h>

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

static struct INotifier *getNotifier(const struct IProxy *self) {
    struct Proxy *this = (struct Proxy *) self;
    return &this->notifier.base;
}

static void onRegister(struct IProxy *self) {
    (void)self;
}

static void onRemove(struct IProxy *self) {
    (void)self;
}

struct IProxy *puremvc_proxy(struct Proxy *const proxy, const char *name, void *data) {
    proxy->base.getName = getName;
    proxy->base.getData = getData;
    proxy->base.setData = setData;
    proxy->base.getNotifier = getNotifier;
    proxy->base.onRegister = onRegister;
    proxy->base.onRemove = onRemove;

    proxy->data = data;
    puremvc_notifier(&proxy->notifier);

    int len = snprintf(proxy->name, NAME_SIZE, "%s", name ? name : PROXY_NAME);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Proxy] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : PROXY_NAME, len, NAME_SIZE);

    return &proxy->base;
}

void puremvc_proxy_deinit(struct Proxy *proxy) {
    proxy->base = (struct IProxy){0};
    memset(&proxy->name, 0, KEY_SIZE);
    proxy->data = NULL;
}
