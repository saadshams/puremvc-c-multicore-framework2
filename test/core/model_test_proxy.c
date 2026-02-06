#include "model_test_proxy.h"

static void onRegister(struct IProxy *self) {
    self->setData(self, ON_REGISTER_CALLED);
}

static void onRemove(struct IProxy *self) {
    self->setData(self, ON_REMOVE_CALLED);
}

struct IProxy *model_test_proxy(struct IProxy *const proxy, const char *name, void *data) {
    struct IProxy *self = puremvc_proxy_init(proxy, name, data);
    proxy->onRegister = onRegister;
    proxy->onRemove = onRemove;
    return self;
}
