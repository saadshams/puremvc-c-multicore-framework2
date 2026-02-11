#include "proxy_test.h"

#include "puremvc/i_proxy.h"

#include <alloca.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test(const char *name, void (*callback)(void)) {
    printf("\033[0;34m[RUNNING]\033[0m %s...\n", name);
    fflush(stdout);

    callback();

    printf("\033[0;32m[PASSED]\033[0m %s\n", name);
    fflush(stdout);
}

int main(void) {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "ProxyTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    test("testConstructor", testConstructor);
    test("testNameAccessors", testNameAccessors);
    test("testDataAccessors", testDataAccessors);
    test("testDataReassign", testDataReassign);
    test("testNotifier", testNotifier);

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

/**
 * Test Constructor
 */
void testConstructor() {
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), NULL, NULL);

    // test assertions
    assert(strcmp(proxy->getName(proxy), PROXY_NAME) == 0);

    assert(proxy->getData(proxy) == NULL);
}

/**
 * Tests getting the name using Proxy class accessor method. Setting can only be done in constructor.
 */
void testNameAccessors() {
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), "TestProxy", NULL);

    // test assertions
    assert(strcmp(proxy->getName(proxy), "TestProxy") == 0);

    const struct IProxy *proxy2 = puremvc_proxy_init(alloca(puremvc_proxy_size()), NULL, NULL);
    assert(strcmp(proxy2->getName(proxy2), PROXY_NAME) == 0);
}

/**
 * Tests setting and getting the item using Proxy class accessor methods.
 */
void testDataAccessors() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), "TestProxy", colors);

    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);
    assert(*(data + 3) == NULL);
}

void testDataReassign() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};

    struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), "colors", colors);

    // Re-assign the same data to ensure the proxy does not free it
    proxy->setData(proxy, colors);

    const char **data = proxy->getData(proxy);

    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);
    assert(*(data + 3) == NULL);
}

void testNotifier() {
    const struct IProxy *proxy = puremvc_proxy_init(alloca(puremvc_proxy_size()), NULL, NULL);
    assert(strcmp(proxy->getName(proxy), PROXY_NAME) == 0);

    proxy->getNotifier(proxy)->initializeNotifier(proxy->getNotifier(proxy), "testing");
}
