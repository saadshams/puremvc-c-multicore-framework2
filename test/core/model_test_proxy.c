#include "model_test_proxy.h"

static void onRegister(struct IProxy *self) {
    self->setData(self, ON_REGISTER_CALLED);
}

static void onRemove(struct IProxy *self) {
    self->setData(self, ON_REMOVE_CALLED);
}

struct IProxy *model_test_proxy(struct Proxy *proxy, const char *name, void *data) {
    struct IProxy *p = puremvc_proxy(proxy, name, data);
    p->onRegister = onRegister;
    p->onRemove = onRemove;
    return p;
}
