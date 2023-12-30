#ifndef PUREMVC_VIEW_TEST_MEDIATOR_H
#define PUREMVC_VIEW_TEST_MEDIATOR_H

#include "mediator.h"
#include "view_test_component.h"

typedef struct ViewTestMediator ViewTestMediator;

struct ViewTestMediator {
    struct Mediator mediator;
};

#define ViewTestMediator_NAME "ViewTestMediator"

ViewTestMediator *test_mediator_init(ViewTestMediator *self);

ViewTestMediator *test_mediator_alloc(ViewComponent *view);

ViewTestMediator *test_mediator_new(ViewComponent *view);

void test_mediator_free(ViewTestMediator **self);

#endif //PUREMVC_VIEW_TEST_MEDIATOR_H
