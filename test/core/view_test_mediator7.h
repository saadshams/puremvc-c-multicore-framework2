#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator7_NAME "Mediator7"

struct IMediator *view_test_mediator7(struct Mediator *mediator, const char *name, struct ViewTest *component);
