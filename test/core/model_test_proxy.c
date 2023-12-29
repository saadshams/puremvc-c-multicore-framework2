#include "model_test_proxy.h"
#include <stdlib.h>
#include <string.h>

static void onRegister(struct Proxy *self) {
    self->data = (void *) ON_REGISTER_CALLED;
}

static void onRemove(struct Proxy *self) {
    self->data = (void *) ON_REMOVE_CALLED;
}

ModelTestProxy *model_test_proxy_init(ModelTestProxy *self) {
    puremvc_proxy_init(&self->proxy);
    self->proxy.onRegister = onRegister;
    self->proxy.onRemove = onRemove;
    return self;
}

ModelTestProxy *model_test_proxy_alloc(const char *proxyName, void *data) {
    ModelTestProxy *self = malloc(sizeof(ModelTestProxy));
    self->proxy.proxyName = strdup(proxyName);
    self->proxy.data = data;
    return self;
}

ModelTestProxy *model_test_proxy_new(const char *proxyName, void *data) {
    return model_test_proxy_init(model_test_proxy_alloc(proxyName, data));
}

void model_test_proxy_free(ModelTestProxy **modelTestProxy) {
    free((void *) (*modelTestProxy)->proxy.proxyName);
    (*modelTestProxy)->proxy.proxyName = NULL;
    (*modelTestProxy)->proxy.data = NULL;
    free(*modelTestProxy);
    *modelTestProxy = NULL;
}
