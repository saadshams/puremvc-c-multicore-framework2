#ifndef PUREMVC_VIEW_TEST_MEDIATOR6_H
#define PUREMVC_VIEW_TEST_MEDIATOR6_H

#include "../../include/mediator.h"
#include "view_test_component.h"

typedef struct ViewTestMediator6 ViewTestMediator6;

struct ViewTestMediator6 {
    struct Mediator mediator;
};

#define ViewTestMediator6_NAME "ViewTestMediator6"

ViewTestMediator6 *test_mediator6_init(ViewTestMediator6 *self);

ViewTestMediator6 *test_mediator6_alloc(const char *mediatorName, ViewTest *view);

ViewTestMediator6 *test_mediator6_new(const char *mediatorName, ViewTest *view);

void test_mediator6_free(ViewTestMediator6 **self);

#endif //PUREMVC_VIEW_TEST_MEDIATOR6_H
