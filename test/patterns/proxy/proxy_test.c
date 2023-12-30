#include "proxy.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/**
 * Node Constructor
 */
void testConstructor() {
    struct Proxy *proxy = puremvc_proxy_new(NULL, NULL);

    // test assertions
    assert(proxy != NULL);
    // assert(proxy->notifier != NULL);
    assert(strcmp(proxy->getProxyName(proxy), PROXY_NAME) == 0);
    puremvc_proxy_free(&proxy);
    assert(proxy == NULL);
}

/**
 * Tests getting the name using Proxy class accessor method. Setting can only be done in constructor.
 */
void testNameAccessors() {
    struct Proxy *proxy = puremvc_proxy_new("TestProxy", NULL);
    // proxy->notifier->initializeNotifier(proxy->notifier, "test");

    // test assertions
    assert(proxy != NULL);
    // assert(proxy->notifier != NULL);
    assert(strcmp(proxy->getProxyName(proxy), "TestProxy") == 0);
    puremvc_proxy_free(&proxy);
    assert (proxy == NULL);

    struct Proxy *proxy2 = puremvc_proxy_new(NULL, NULL);
    puremvc_proxy_free(&proxy2);
    assert (proxy2 == NULL);

    struct Proxy *proxy3 = puremvc_proxy_new(NULL, NULL);
    puremvc_proxy_free(&proxy3);
    assert (proxy3 == NULL);
}

/**
 * Tests setting and getting the item using Proxy class accessor methods.
 */
void testDataAccessors() {
    // Create a new Proxy and use accessors to set the item
    struct Proxy *proxy = puremvc_proxy_new("colors", NULL);
    // proxy->notifier->initializeNotifier(proxy->notifier, "test");

    assert(proxy != NULL);
    proxy->setData(proxy, (const char *[]) {"red", "green", "blue", NULL});

    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);
    assert(*(data + 3) == NULL);

    puremvc_proxy_free(&proxy);
    assert (proxy == NULL);
}

/**
 * Tests setting and getting the dynamic item using Proxy class accessor methods.
 * Code responsible for passing the item to the proxy is responsible to free any
 * dynamically allocated item since Proxy has no way of knowing the type of allocation
 */
void testDynamicDataAccessors() {
    char *source = "{red, green, blue}";
    char *values = (char *) malloc((strlen(source) + 1) * sizeof(char)); // dynamic
    strcpy(values, source);
    struct Proxy *proxy = puremvc_proxy_new("colors", values);

    const char **data = proxy->getData(proxy);
    free(data); // code responsible for allocation needs to free item itself
    data = NULL;
    puremvc_proxy_free(&proxy);
    assert (proxy == NULL);
}

/**
 * Tests Proxy class destructor method.
 */
void testDestructor() {
    struct Proxy *proxy = puremvc_proxy_new("TestProxy", NULL);

    assert(proxy != NULL);
    puremvc_proxy_free(&proxy);
    assert (proxy == NULL);
}

int main() {
    testConstructor();
    testNameAccessors();
    testDataAccessors();
    testDynamicDataAccessors();
    testDestructor();
    return 0;
}
