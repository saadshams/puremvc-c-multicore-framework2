#include "proxy_test.h"

#include "puremvc/platform.h"
#include "puremvc/i_proxy.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void beforeAll() {}

static void beforeEach() {}

static void afterEach() {}

static void afterAll() {}

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    beforeEach();
    callback();
    afterEach();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main(void) {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "ProxyTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    beforeAll();
    test("testConstructor", testConstructor);
    test("testNameAccessors", testNameAccessors);
    test("testDataAccessors", testDataAccessors);
    test("testDataReassign", testDataReassign);
    test("testNotifier", testNotifier);
    afterAll();

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

/**
 * Test Constructor
 */
void testConstructor() {
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), NULL, NULL);

    // test assertions
    if (strcmp(proxy->getName(proxy), PROXY_NAME) != 0) abort();

    if (proxy->getData(proxy) != NULL) abort();
}

/**
 * Tests getting the name using Proxy class accessor method. Setting can only be done in constructor.
 */
void testNameAccessors() {
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), "TestProxy", NULL);

    // test assertions
    if (strcmp(proxy->getName(proxy), "TestProxy") != 0) abort();

    const struct IProxy *proxy2 = puremvc_proxy_init(alloca(puremvc_proxy_size()), NULL, NULL);
    if (strcmp(proxy2->getName(proxy2), PROXY_NAME) != 0) abort();
}

/**
 * Tests setting and getting the item using Proxy class accessor methods.
 */
void testDataAccessors() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), "TestProxy", colors);

    const char **data = proxy->getData(proxy);

    // test assertions
    if (strcmp(*data, "red") != 0) abort();
    if (strcmp(*(data + 1), "green") != 0) abort();
    if (strcmp(*(data + 2), "blue") != 0) abort();
    if (*(data + 3) != NULL) abort();
}

void testDataReassign() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};

    struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), "colors", colors);

    // Re-assign the same data to ensure the proxy does not free it
    proxy->setData(proxy, colors);

    const char **data = proxy->getData(proxy);

    if (strcmp(*data, "red") != 0) abort();
    if (strcmp(*(data + 1), "green") != 0) abort();
    if (strcmp(*(data + 2), "blue") != 0) abort();
    if (*(data + 3) != NULL) abort();
}

void testNotifier() {
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), NULL, NULL);
    if (strcmp(proxy->getName(proxy), PROXY_NAME) != 0) abort();

    proxy->getNotifier(proxy)->initializeNotifier(proxy->getNotifier(proxy), "testing");
}
