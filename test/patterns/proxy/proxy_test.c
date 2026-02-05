#include "proxy_test.h"
#include "puremvc/proxy.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

int main(void) {
    testConstructor();
    testNameAccessors();
    testDataAccessors();
    testDataReassign();
    testNotifier();
    return 0;
}

/**
 * Test Constructor
 */
void testConstructor() {
    const struct IProxy *proxy = puremvc_proxy(&(struct Proxy){0}, NULL, NULL);

    // test assertions
    assert(strcmp(proxy->getName(proxy), PROXY_NAME) == 0);

    assert(proxy->getData(proxy) == NULL);
}

/**
 * Tests getting the name using Proxy class accessor method. Setting can only be done in constructor.
 */
void testNameAccessors() {
    const struct IProxy *proxy = puremvc_proxy(&(struct Proxy){0}, "TestProxy", NULL);

    // test assertions
    assert(strcmp(proxy->getName(proxy), "TestProxy") == 0);

    const struct IProxy *proxy2 = puremvc_proxy(&(struct Proxy){0}, NULL, NULL);
    assert(strcmp(proxy2->getName(proxy2), PROXY_NAME) == 0);
}

/**
 * Tests setting and getting the item using Proxy class accessor methods.
 */
void testDataAccessors() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    const struct IProxy *proxy = puremvc_proxy(&(struct Proxy){0}, "colors", colors);

    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);
    assert(*(data + 3) == NULL);
}

void testDataReassign() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};

    struct IProxy *proxy = puremvc_proxy(&(struct Proxy){0}, "colors", colors);

    // Re-assign the same data to ensure the proxy does not free it
    proxy->setData(proxy, colors);

    const char **data = proxy->getData(proxy);

    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);
    assert(*(data + 3) == NULL);
}

void testNotifier() {
    const struct IProxy *proxy = puremvc_proxy(&(struct Proxy){}, NULL, NULL);
    assert(strcmp(proxy->getName(proxy), PROXY_NAME) == 0);

    proxy->getNotifier(proxy)->initializeNotifier(proxy->getNotifier(proxy), "testing");
}
