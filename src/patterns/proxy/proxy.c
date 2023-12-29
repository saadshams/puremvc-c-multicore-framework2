#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "proxy.h"
#include "notifier.h"

static const char *getProxyName(const struct Proxy *self) {
    return self->proxyName;
}

static void *getData(const struct Proxy *self) {
    return self->data;
}

static void setData(struct Proxy *self, void *data) {
    self->data = data;
}

static void onRegister(struct Proxy *proxy) {

}

static void onRemove(struct Proxy *proxy) {

}

struct Proxy *puremvc_proxy_init(struct Proxy *self) {
    self->notifier = puremvc_notifier_new();
    self->getProxyName = getProxyName;
    self->getData = getData;
    self->setData = setData;
    self->onRegister = onRegister;
    self->onRemove = onRemove;
    return self;
}

struct Proxy *puremvc_proxy_alloc(const char *proxyName, void *data) {
    struct Proxy *self = malloc(sizeof(struct Proxy));
    if (self == NULL) goto exception;
    memset(self, 0, sizeof(*self));

    self->proxyName = proxyName != NULL ? strdup(proxyName) : strdup(PROXY_NAME);
    if (self->proxyName == NULL) {
        free(self);
        goto exception;
    }
    self->data = data;
    return self;

    exception:
    fprintf(stderr, "Proxy allocation failed.\n");
    return NULL;
}

struct Proxy *puremvc_proxy_new(const char *proxyName, void *data) {
    return puremvc_proxy_init(puremvc_proxy_alloc(proxyName, data));
}

void puremvc_proxy_free(struct Proxy **self) {
    puremvc_notifier_free(&(*self)->notifier);
    free((void *) (*self)->proxyName);
    (*self)->proxyName = NULL;
    (*self)->data = NULL;
    free(*self);
    *self = NULL;
}
