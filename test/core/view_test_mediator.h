#pragma once

#include "puremvc/i_mediator.h"

#define view_test_mediator_NAME "Mediator1"

struct IMediator *view_test_mediator(void *buffer, const char *name, void *component);
