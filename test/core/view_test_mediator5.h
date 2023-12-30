#ifndef PUREMVC_VIEW_TEST_MEDIATOR5_H
#define PUREMVC_VIEW_TEST_MEDIATOR5_H

#include "mediator.h"
#include "view_test_component.h"

typedef struct ViewTestMediator5 ViewTestMediator5;

struct ViewTestMediator5 {
    struct Mediator mediator;
};

#define ViewTestMediator5_NAME "ViewTestMediator5"

ViewTestMediator5 *test_mediator5_init(ViewTestMediator5 *self);

ViewTestMediator5 *test_mediator5_alloc(ViewTest *view);

ViewTestMediator5 *test_mediator5_new(ViewTest *view);

void test_mediator5_free(ViewTestMediator5 **self);

#endif //PUREMVC_VIEW_TEST_MEDIATOR5_H
