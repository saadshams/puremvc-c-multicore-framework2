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

struct IProxy *puremvc_proxy_init(struct IProxy *const proxy, const char *name, void *data) {
    struct Proxy *this = (struct Proxy *) proxy;

    proxy->getName = getName;
    proxy->getData = getData;
    proxy->setData = setData;
    proxy->getNotifier = getNotifier;
    proxy->onRegister = onRegister;
    proxy->onRemove = onRemove;

    int len = snprintf(this->name, NAME_SIZE, "%s", name ? name : PROXY_NAME);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Proxy] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : PROXY_NAME, len, NAME_SIZE);

    this->data = data;

    puremvc_notifier_init(&this->notifier);

    return proxy;
}
