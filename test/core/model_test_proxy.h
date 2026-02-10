#pragma once

#include "puremvc/i_proxy.h"

#define ON_REGISTER_CALLED "onRegister Called"
#define ON_REMOVE_CALLED "onRemove Called"

struct IProxy *model_test_proxy(void *buffer, const char *name, void *data);
