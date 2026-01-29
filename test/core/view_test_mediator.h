#pragma once

#include "puremvc/mediator.h"
#include "view_test.h"

#define view_test_mediator_NAME "Mediator"

struct Mediator view_test_mediator(struct ViewComponent *component);
