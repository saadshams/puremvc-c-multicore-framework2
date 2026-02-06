#pragma once

#include "puremvc/mediator.h"

#define view_test_mediator3_NAME "Mediator3"

struct IMediator *view_test_mediator3(struct IMediator *mediator, const char *name, void *component);
