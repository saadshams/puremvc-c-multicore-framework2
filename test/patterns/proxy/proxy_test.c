#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "proxy_test.h"
#include "puremvc/proxy.h"

int main(void) {
    testConstructor();
    testNameAccessors();
    testDataAccessors();
    testDataReassign();
    return 0;
}

/**
 * Test Constructor
 */
void testConstructor() {
    // struct Proxy myProxy = {0};
    const struct Proxy proxy = puremvc_proxy(NULL, NULL);
    const struct IProxy *p = &proxy.base;

    // test assertions
    assert(strcmp(p->getName(p), PROXY_NAME) == 0);

    assert(p->getData(p) == NULL);
}

/**
 * Tests getting the name using Proxy class accessor method. Setting can only be done in constructor.
 */
void testNameAccessors() {
    const struct Proxy proxy = puremvc_proxy("TestProxy", NULL);
    const struct IProxy *p = &proxy.base;

    // test assertions
    assert(strcmp(p->getName(p), "TestProxy") == 0);

    struct Proxy proxy2 = puremvc_proxy(NULL, NULL);
    const struct IProxy *p2 = &proxy2.base;
    assert(strcmp(p2->getName(p2), PROXY_NAME) == 0);
}

/**
 * Tests setting and getting the item using Proxy class accessor methods.
 */
void testDataAccessors() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    const struct Proxy proxy = puremvc_proxy("colors", colors);
    const struct IProxy *p = &proxy.base;

    const char **data = p->getData(p);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);
    assert(*(data + 3) == NULL);
}

void testDataReassign() {
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};

    struct Proxy proxy = puremvc_proxy("colors", colors);
    const struct IProxy *p = &proxy.base;

    // Re-assign the same data to ensure the proxy does not free it
    p->setData(p, colors);

    const char **data = p->getData(p);

    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);
    assert(*(data + 3) == NULL);
}
