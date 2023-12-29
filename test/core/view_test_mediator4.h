#ifndef PUREMVC_VIEW_TEST_MEDIATOR4_H
#define PUREMVC_VIEW_TEST_MEDIATOR4_H

#include "../../include/mediator.h"
#include "view_test_component.h"

typedef struct ViewTestMediator4 ViewTestMediator4;

struct ViewTestMediator4 {
    struct Mediator mediator;
};

#define ViewTestMediator4_NAME "ViewTestMediator4"

ViewTestMediator4 *test_mediator4_init(ViewTestMediator4 *self);

ViewTestMediator4 *test_mediator4_alloc(ViewTest *view);

ViewTestMediator4 *test_mediator4_new(ViewTest *view);

void test_mediator4_free(ViewTestMediator4 **self);

#endif //PUREMVC_VIEW_TEST_MEDIATOR4_H
