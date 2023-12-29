#include "../../../include/mediator.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Node Constructor
 */
void testConstructor() {
    struct Mediator *mediator = puremvc_mediator_new(NULL, NULL);

    // test assertions
    assert(mediator != NULL);
    // assert(mediator->notifier != NULL);
    assert(strcmp(mediator->mediatorName, MEDIATOR_NAME) == 0);
    puremvc_mediator_free(&mediator);
    assert(mediator == NULL);
}

/**
 * Tests getting the name using Mediator class accessor method.
 */
void testNameAccessor() {
    struct Mediator *mediator = puremvc_mediator_new("TestMediator", NULL);
    // mediator->notifier->initializeNotifier(mediator->notifier, "test");

    // test assertions
    assert(mediator != NULL);
    // assert(mediator->notifier != NULL);
    assert(strcmp(mediator->mediatorName, "TestMediator") == 0);
    puremvc_mediator_free(&mediator);
    assert(mediator == NULL);

    struct Mediator *mediator2 = puremvc_mediator_new(NULL, NULL);
    puremvc_mediator_free(&mediator2);
    assert(mediator2 == NULL);

    struct Mediator *mediator3 = puremvc_mediator_new(NULL, NULL);
    puremvc_mediator_free(&mediator3);
    assert(mediator3 == NULL);
}

/**
 * Tests getting the viewComponent using Mediator class accessor method.
 */
void testViewAccessor() {
    // Create a view object
    struct View {} view;
    struct Mediator *mediator = puremvc_mediator_new(MEDIATOR_NAME, &view);
    // mediator->notifier->initializeNotifier(mediator->notifier, "test");

    // test assertions
    assert(mediator->viewComponent == &view);
    mediator->viewComponent = NULL;
    assert(mediator->viewComponent == NULL);
    puremvc_mediator_free(&mediator);
    assert(mediator == NULL);
}

/**
 * Tests setting and getting the dynamic viewComponent using Mediator class accessor methods.
 * Code responsible for passing the viewComponent to the Mediator is responsible to free any
 * dynamically allocated item since Mediator has no way of knowing the type of allocation
 */
void testDynamicViewAccessor() {
    struct View{};
    struct Mediator *mediator = puremvc_mediator_new(MEDIATOR_NAME, malloc(sizeof(struct View)));

    struct View *view = mediator->viewComponent;
    free(view); // code responsible for allocation needs to free the allocation itself
    view = NULL;
    puremvc_mediator_free(&mediator);
    assert(mediator == NULL);
}

/**
 * Tests Mediator class destructor method.
 */
void testDestructor() {
    struct Mediator *mediator = puremvc_mediator_new("TestProxy", NULL);

    assert(mediator != NULL);
    puremvc_mediator_free(&mediator);
    assert (mediator == NULL);
}

int main() {
    testConstructor();
    testNameAccessor();
    testViewAccessor();
    testDynamicViewAccessor();
    testDestructor();
    return 0;
}
