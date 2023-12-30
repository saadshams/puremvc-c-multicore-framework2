#ifndef PUREMVC_VIEW_TEST_MEDIATOR3_H
#define PUREMVC_VIEW_TEST_MEDIATOR3_H

#include "mediator.h"
#include "view_test_component.h"

typedef struct ViewTestMediator3 ViewTestMediator3;

struct ViewTestMediator3 {
    struct Mediator mediator;
};

#define ViewTestMediator3_NAME "ViewTestMediator3"

ViewTestMediator3 *test_mediator3_init(ViewTestMediator3 *self);

ViewTestMediator3 *test_mediator3_alloc(ViewTest *view);

ViewTestMediator3 *test_mediator3_new(ViewTest *view);

void test_mediator3_free(ViewTestMediator3 **self);

#endif //PUREMVC_VIEW_TEST_MEDIATOR3_H
