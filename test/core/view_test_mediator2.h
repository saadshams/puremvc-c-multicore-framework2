#pragma once

#include "puremvc/mediator.h"

#define view_test_mediator2_NAME "Mediator2"

struct IMediator *view_test_mediator2(struct IMediator *mediator, const char *name, void *component);
