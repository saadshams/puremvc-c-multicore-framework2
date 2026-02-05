#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator5_NAME "Mediator5"

struct IMediator *view_test_mediator5(struct Mediator *mediator, struct ViewTest *component);
