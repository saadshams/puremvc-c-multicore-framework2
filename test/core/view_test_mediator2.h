#ifndef PUREMVC_VIEW_TEST_MEDIATOR2_H
#define PUREMVC_VIEW_TEST_MEDIATOR2_H

#include "../../include/mediator.h"
#include "view_test_component.h"

typedef struct ViewTestMediator2 ViewTestMediator2;

struct ViewTestMediator2 {
    struct Mediator mediator;
};

#define ViewTestMediator2_NAME "ViewTestMediator2"

ViewTestMediator2 *test_mediator2_init(ViewTestMediator2 *self);

ViewTestMediator2 *test_mediator2_alloc(ViewTest *view);

ViewTestMediator2 *test_mediator2_new(ViewTest *view);

void test_mediator2_free(ViewTestMediator2 **self);

#endif //PUREMVC_VIEW_TEST_MEDIATOR2_H
