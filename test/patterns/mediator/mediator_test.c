#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "mediator_test.h"
#include "puremvc/mediator.h"

int main() {
    testConstructor();
    testNameAccessor();
    testViewAccessor();
    return 0;
}

/**
 * Test Constructor
 */
void testConstructor() {
    const struct Mediator m = puremvc_mediator(NULL, NULL);
    const struct IMediator *mediator = &m.base;

    // test assertions
    assert(strcmp(mediator->getName(mediator), MEDIATOR_NAME) == 0);
}

/**
 * Tests getting the name using Mediator class accessor method.
 */
void testNameAccessor() {
    const struct Mediator m = puremvc_mediator("TestMediator", NULL);
    const struct IMediator *mediator = &m.base;

    // test assertions
    assert(strcmp(mediator->getName(mediator), "TestMediator") == 0);
}

/**
 * Tests getting the viewComponent using Mediator class accessor method.
 */
void testViewAccessor() {
    // Create a view object
    struct Component {int x;} component;
    struct Mediator m = puremvc_mediator(MEDIATOR_NAME, &component);
    struct IMediator *mediator = &m.base;

    // test assertions
    assert(mediator->getComponent(mediator) == &component);
    mediator->setComponent(mediator, NULL);
    assert(mediator->getComponent(mediator) == NULL);
}
