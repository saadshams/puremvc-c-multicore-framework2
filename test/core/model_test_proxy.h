#pragma once

#include "puremvc/proxy.h"

#define ON_REGISTER_CALLED "onRegister Called"
#define ON_REMOVE_CALLED "onRemove Called"

struct IProxy *model_test_proxy(struct Proxy *proxy, const char *name, void *data);
