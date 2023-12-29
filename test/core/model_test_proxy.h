#ifndef PUREMVC_MODEL_TEST_PROXY_H
#define PUREMVC_MODEL_TEST_PROXY_H

#include "../../include/proxy.h"

#define ON_REGISTER_CALLED "onRegister Called"
#define ON_REMOVE_CALLED "onRemove Called"

typedef struct ModelTestProxy ModelTestProxy;

struct ModelTestProxy {
    struct Proxy proxy;
};

ModelTestProxy *model_test_proxy_init(ModelTestProxy *self);

ModelTestProxy *model_test_proxy_alloc(const char *proxyName, void *data);

ModelTestProxy *model_test_proxy_new(const char *, void *);

void model_test_proxy_free(ModelTestProxy **modelTestProxy);

#endif //PUREMVC_MODEL_TEST_PROXY_H
