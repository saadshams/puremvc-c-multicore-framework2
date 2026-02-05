#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator3_NAME "Mediator3"

struct IMediator *view_test_mediator3(struct Mediator *mediator, struct ViewTest *component);
