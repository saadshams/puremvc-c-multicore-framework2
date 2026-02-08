#include "mediator_test.h"
#include "puremvc/i_mediator.h"

#include <alloca.h>
#include <assert.h>
// #include <stdio.h>
#include <string.h>

int main() {
    testConstructor();
    testNameAccessor();
    testViewAccessor();
    testNotifier();
    return 0;
}

/**
 * Test Constructor
 */
void testConstructor() {
    const struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size(NULL)), NULL, NULL);

    // test assertions
    assert(strcmp(mediator->getName(mediator), MEDIATOR_NAME) == 0);
}

/**
 * Tests getting the name using Mediator class accessor method.
 */
void testNameAccessor() {
    const struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size("TestMediator")), "TestMediator", NULL);

    // test assertions
    assert(strcmp(mediator->getName(mediator), "TestMediator") == 0);
}

/**
 * Tests getting the viewComponent using Mediator class accessor method.
 */
void testViewAccessor() {
    // Create a view object
    struct Component {int x;} component;
    struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size(MEDIATOR_NAME)), MEDIATOR_NAME, &component);

    // test assertions
    assert(mediator->getComponent(mediator) == &component);
    mediator->setComponent(mediator, NULL);
    assert(mediator->getComponent(mediator) == NULL);
}

void testNotifier() {
    struct Component {int x;} component;
    const struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size(MEDIATOR_NAME)), MEDIATOR_NAME, &component);

    mediator->getNotifier(mediator)->initializeNotifier(mediator->getNotifier(mediator), "testing");
}
