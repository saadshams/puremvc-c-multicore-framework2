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

size_t puremvc_proxy_size() {
    return (sizeof(struct Proxy) + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1);
}

struct IProxy *puremvc_proxy_init(void *buffer, const char *name, void *data) {
    struct Proxy *this = (struct Proxy *) buffer;

    memset(this, 0, sizeof(struct Proxy));

    this->base.getName = getName;
    this->base.getData = getData;
    this->base.setData = setData;
    this->base.getNotifier = getNotifier;
    this->base.onRegister = onRegister;
    this->base.onRemove = onRemove;

    const char *key = name != NULL ? name : PROXY_NAME;
    int len = snprintf(this->name, sizeof(this->name), "%s", key);
    if (len >= (int) sizeof(this->name)) {
        memset(this, 0, sizeof(struct Proxy));
        fprintf(stderr, "\033[0;31m[PureMVC::Proxy::init] Error: Proxy name truncated: '%s' (max %zu chars).\033[0m\n", key, sizeof(this->name));
        return NULL;
    }

    this->data = data;

    puremvc_notifier_init((struct INotifier *) &this->notifier);

    return (struct IProxy *) this;
}
