#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator2_NAME "Mediator2"

struct IMediator *view_test_mediator2(struct Mediator *const mediator, struct ViewTest *component);
