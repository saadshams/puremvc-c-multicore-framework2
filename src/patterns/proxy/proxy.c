/**
* @file proxy.c
* @internal
* @brief Proxy Implementation
*
* @author Saad Shams <saad.shams@puremvc.org>
* @copyright BSD 3-Clause License
*/
#include "proxy.h"

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
    return (sizeof(struct Proxy) + len + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1); // align to pointer size
}

struct IProxy *puremvc_proxy_init(void *buffer, const char *name, void *data) {
    struct Proxy *this = (struct Proxy *) buffer;

    memset(this, 0, sizeof *this);

    this->base.getName = getName;
    this->base.getData = getData;
    this->base.setData = setData;
    this->base.getNotifier = getNotifier;
    this->base.onRegister = onRegister;
    this->base.onRemove = onRemove;

    this->data = data;

    this->name_len = strlen(name != NULL ? name : PROXY_NAME) + 1;
    snprintf(this->name, this->name_len, "%s", name != NULL ? name : PROXY_NAME);

    puremvc_notifier_init((struct INotifier *) &this->notifier);

    return (struct IProxy *) this;
}
