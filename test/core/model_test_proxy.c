#include "model_test_proxy.h"

static void onRegister(struct IProxy *self) {
    self->setData(self, ON_REGISTER_CALLED);
}

static void onRemove(struct IProxy *self) {
    self->setData(self, ON_REMOVE_CALLED);
}

struct IProxy *model_test_proxy(void *buffer, const char *name, void *data) {
    struct IProxy *proxy = puremvc_proxy_init(buffer, name, data);
    proxy->onRegister = onRegister;
    proxy->onRemove = onRemove;
    return proxy;
}
