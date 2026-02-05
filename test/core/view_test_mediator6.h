#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator6_NAME "Mediator6"

struct IMediator *view_test_mediator6(struct Mediator *mediator, const char *name, struct ViewTest *component);
