/**
* @file proxy.c
* @internal
* @brief Proxy Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "proxy.h"
#include "puremvc/i_notifier.h"

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
    return (struct INotifier *) &this->notifier;
}

static void onRegister(struct IProxy *self) {
    (void)self;
}

static void onRemove(struct IProxy *self) {
    (void)self;
}

size_t puremvc_proxy_size(const char *name) {
    const size_t len = strlen(name ? name : PROXY_NAME) + 1;
    // Round up to the nearest multiple of a pointer size (e.g., 8 bytes)
    return (sizeof(struct Proxy) + len + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct IProxy *puremvc_proxy_init(void *buffer, const char *name, void *data) {
    struct Proxy *this = (struct Proxy *) buffer;
    struct IProxy *proxy = (struct IProxy *) buffer;

    memset(this, 0, sizeof *this);

    proxy->getName = getName;
    proxy->getData = getData;
    proxy->setData = setData;
    proxy->getNotifier = getNotifier;
    proxy->onRegister = onRegister;
    proxy->onRemove = onRemove;

    memset(&this->name, 0, NAME_SIZE);
    int len = snprintf(this->name, NAME_SIZE, "%s", name ? name : PROXY_NAME);
    if (len >= NAME_SIZE)
        fprintf(stderr, "[PureMVC::Proxy] Warning: Name Truncated: '%s' (Original length: %d, Buffer size: %d)\n", name ? name : PROXY_NAME, len, NAME_SIZE);

    this->data = data;

    puremvc_notifier_init((struct INotifier *) &this->notifier);

    return proxy;
}
