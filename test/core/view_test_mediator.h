#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator_NAME "Mediator"

struct IMediator *view_test_mediator(struct Mediator *mediator, struct ViewComponent *component);
