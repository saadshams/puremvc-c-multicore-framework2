#pragma once

#include "puremvc/proxy.h"

#define ON_REGISTER_CALLED "onRegister Called"
#define ON_REMOVE_CALLED "onRemove Called"

struct Proxy model_test_proxy(const char *name, void *data);
