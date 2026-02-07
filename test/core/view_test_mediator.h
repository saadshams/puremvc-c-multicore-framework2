#pragma once

#include "puremvc/mediator.h"

#define view_test_mediator_NAME "Mediator1"

struct IMediator *view_test_mediator(struct IMediator *mediator, const char *name, void *component);
