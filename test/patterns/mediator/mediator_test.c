#include "mediator_test.h"

#include "puremvc/platform.h"
#include "puremvc/i_mediator.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    callback();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "MediatorTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testConstructor", testConstructor);
    test("testNameAccessor", testNameAccessor);
    test("testViewAccessor", testViewAccessor);
    test("testNotifier", testNotifier);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

/**
 * Test Constructor
 */
void testConstructor() {
    const struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size()), NULL, NULL);

    // test assertions
    if (strcmp(mediator->getName(mediator), MEDIATOR_NAME) != 0) abort();
}

/**
 * Tests getting the name using Mediator class accessor method.
 */
void testNameAccessor() {
    const struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size()), "TestMediator", NULL);

    // test assertions
    if (strcmp(mediator->getName(mediator), "TestMediator") != 0) abort();
}

/**
 * Tests getting the viewComponent using Mediator class accessor method.
 */
void testViewAccessor() {
    // Create a view object
    struct Component {int x;} component;
    struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size()), MEDIATOR_NAME, &component);

    // test assertions
    if (mediator->getComponent(mediator) != &component) abort();
    mediator->setComponent(mediator, NULL);
    if (mediator->getComponent(mediator) != NULL) abort();
}

void testNotifier() {
    struct Component {int x;} component;
    const struct IMediator *mediator = puremvc_mediator_init(alloca(puremvc_mediator_size()), MEDIATOR_NAME, &component);

    mediator->getNotifier(mediator)->initializeNotifier(mediator->getNotifier(mediator), "testing");
}
