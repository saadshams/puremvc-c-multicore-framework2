#include <string.h>

#include "model_test_proxy.h"

static void onRegister(struct Proxy *self) {
    self->setData(self, ON_REGISTER_CALLED);
}

static void onRemove(struct Proxy *self) {
    self->setData(self, ON_REMOVE_CALLED);
}

struct Proxy model_test_proxy(const char *name, void *data) {
    struct Proxy proxy = puremvc_proxy(name, data);
    proxy.onRegister = onRegister;
    proxy.onRemove = onRemove;
    return proxy;
}
