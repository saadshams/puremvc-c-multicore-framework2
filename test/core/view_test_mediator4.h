#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator4_NAME "Mediator4"

struct IMediator *view_test_mediator4(struct Mediator *mediator, struct ViewTest *component);
