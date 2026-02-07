#include <assert.h>
#include <string.h>

#include "model_test.h"
#include "puremvc/model.h"

#include "model_test_proxy.h"

int main() {
    testGetInstance();
    testRegisterAndRetrieveProxy();
    testHasProxy();
    testRegisterAndRemoveProxy();
    testOnRegisterAndOnRemove();
    testRemoveModel();
    testRegisterAndReplaceProxy();
    testRegisterAndRemoveMultipleProxies();
    TestModelShiftLeft();
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct ModelMap *storage[] = { &(struct ModelMap) { // supply empty key storage
        .key = "",
        .model = { .multitonKey = "", .proxyMap = (struct ProxyMap*[]) {}, }
    }, NULL };

    const struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey1");

    // test assertions
    assert(model != NULL);
    assert(model == puremvc_model_getInstance(storage, "ModelTestKey1"));
    assert(strcmp(storage[0]->key, "ModelTestKey1") == 0);
    assert(strcmp(storage[0]->model.multitonKey, "ModelTestKey1") == 0);

    assert(puremvc_model_getInstance(NULL, "ModelTestKey1") == NULL);

    puremvc_model_removeModel(storage, "ModelTestKey1");
    assert(storage[0]->key[0] == '\0');
    model = NULL;
}

void testRegisterAndRetrieveProxy() {
    struct ModelMap *storage[] = { &(struct ModelMap) { // supply empty key storage with one slot for proxy
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base }, NULL } }
    }, NULL };

    // register a proxy and retrieve it.
    struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey2");

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    model->registerProxy(model, puremvc_proxy_init, "colors", colors);
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "colors") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "colors") == 0);

    // Retrieve the proxy
    const struct IProxy *proxy = model->retrieveProxy(model, "colors");

    // test assertions
    assert(proxy != NULL);
    assert(proxy->getData(proxy) == colors);
    const char **data = proxy->getData(proxy);
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    struct IProxy *removedProxy = &(struct Proxy){0}.base;
    model->removeProxy(model, removedProxy, "colors");
    assert(storage[0]->model.proxyMap[0]->key[0] == '\0');
    assert(strcmp(removedProxy->getName(removedProxy), "colors") == 0);

    model->retrieveProxy(model, "colors");

    puremvc_model_removeModel(storage, "ModelTestKey2");
    model = NULL;
}

void testHasProxy() {
    struct ModelMap *storage[] = { &(struct ModelMap) {
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base }, NULL } }
    }, NULL };

    // Get the Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey3");

    // Create and register the test mediator
    const char **aces = (const char *[]) {"clubs", "spades", "hearts", "diamonds", NULL};
    model->registerProxy(model, puremvc_proxy_init, "aces", aces);
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "aces") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "aces") == 0);

    // assert that the model.hasProxy method returns true
    // for that proxy name
    assert(model->hasProxy(model, "aces") == true);

    // remove the proxy
    struct IProxy *removedProxy = &(struct Proxy){0}.base;
    model->removeProxy(model, removedProxy, "aces");
    assert(storage[0]->model.proxyMap[0]->key[0] == '\0');
    assert(strcmp(removedProxy->getName(removedProxy), "aces") == 0);

    // assert that the model.hasProxy method returns false
    // for that proxy name
    assert(model->hasProxy(model, "aces") == false);

    puremvc_model_removeModel(storage, "ModelTestKey3");
    model = NULL;
}

void testRegisterAndRemoveProxy() {
    struct ModelMap *storage[] = { &(struct ModelMap) {
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base }, NULL } }
    }, NULL };

    // Get the Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey4");

    int *sizes = (int []) {1, 2, 3, 0};

    // Register the proxy
    model->registerProxy(model, puremvc_proxy_init, "sizes", sizes);
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "sizes") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "sizes") == 0);

    // remove the proxy
    struct IProxy *removedProxy = &(struct Proxy){0}.base;
    model->removeProxy(model, removedProxy, "sizes");

    // assert that we removed the appropriate proxy
    assert(strcmp(removedProxy->getName(removedProxy), "sizes") == 0);

    // ensure that the proxy is no longer retrievable from the model
    assert(model->retrieveProxy(model, "sizes") == NULL);

    puremvc_model_removeModel(storage, "ModelTestKey4");
    model = NULL;
}

void testOnRegisterAndOnRemove() {
    struct ModelMap *storage[] = { &(struct ModelMap) {
        .model = { .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base }, NULL } }
    }, NULL };

    // Get a Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey5");

    // Create and register the test proxy
    model->registerProxy(model, model_test_proxy, "ModelTestProxy", NULL);
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "ModelTestProxy") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "ModelTestProxy") == 0);

    // assert that onRegister was called, and the proxy responded by setting its data accordingly
    const struct IProxy *proxy = model->retrieveProxy(model, "ModelTestProxy");
    assert(strcmp(proxy->getData(proxy), ON_REGISTER_CALLED) == 0);

    // Remove the proxy
    struct IProxy *removedProxy = &(struct Proxy){0}.base;
    model->removeProxy(model, removedProxy, "ModelTestProxy");
    assert(strcmp(removedProxy->getName(removedProxy), "ModelTestProxy") == 0);

    // assert that onRemove was called, and the proxy responded by setting its data accordingly
    assert(strcmp(removedProxy->getData(removedProxy), ON_REMOVE_CALLED) == 0);

    puremvc_model_removeModel(storage, "ModelTestKey5");
    model = NULL;
}

void testRemoveModel() {
    struct ModelMap *storage[] = { &(struct ModelMap) {}, NULL };

    // Get a Multiton Model instance
    puremvc_model_getInstance(storage, "ModelTestKey6");

    assert(strcmp(storage[0]->key, "ModelTestKey6") == 0);
    assert(strcmp(storage[0]->model.multitonKey, "ModelTestKey6") == 0);

    // remove the model
    puremvc_model_removeModel(storage, "ModelTestKey6");
    assert(storage[0]->key[0] == '\0');

    // re-create the model without throwing an exception
    puremvc_model_getInstance(storage, "ModelTestKey6");
    assert(strcmp(storage[0]->key, "ModelTestKey6") == 0);
    assert(strcmp(storage[0]->model.multitonKey, "ModelTestKey6") == 0);

    // try removing again
    puremvc_model_removeModel(storage, "ModelTestKey6");
    assert(storage[0]->key[0] == '\0');
}

void testRegisterAndReplaceProxy() {
    struct ModelMap *storage[] = { &(struct ModelMap) { // empty model key storage with two slots for proxy
        .model = {
            .proxyMap = (struct ProxyMap*[]) { &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base },
                &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base }, NULL }
        }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey9");

    int *sizes = (int []) {1, 0};
    assert(model->registerProxy(model, puremvc_proxy_init, "sizes", sizes) == true);

    // replace with another proxy
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    assert(model->registerProxy(model, puremvc_proxy_init, "sizes", colors) == true);

    // try to retrieve the replaced proxy
    const struct IProxy *proxy = model->retrieveProxy(model, "sizes");

    // test assertions
    assert(proxy != NULL);
    const char **data = proxy->getData(proxy);
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    struct IProxy *removedProxy = &(struct Proxy){0}.base;
    model->removeProxy(model, removedProxy, "sizes");
    assert(strcmp(removedProxy->getName(removedProxy), "sizes") == 0);

    assert(model->retrieveProxy(model, "sizes") == NULL);
    puremvc_model_removeModel(storage, "ModelTestKey9");
    model = NULL;
}

void testRegisterAndRemoveMultipleProxies() {
    struct ModelMap *storage[] = { &(struct ModelMap) { // empty key model storage with four slots for proxy
        .model = {
            .proxyMap = (struct ProxyMap*[]) {
                &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base }, &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base },
                &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base }, &(struct ProxyMap){ .proxy = &(struct Proxy){0}.base },
                NULL
            }
        }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(storage, "ModelTestKey10");

    // Register four proxies and verify that each is correctly associated to their dictionaries
    model->registerProxy(model, puremvc_proxy_init, "proxy0", NULL);
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "proxy0") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "proxy0") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy1", NULL);
    assert(strcmp(storage[0]->model.proxyMap[1]->key, "proxy1") == 0);
    assert(strcmp(storage[0]->model.proxyMap[1]->proxy->getName(storage[0]->model.proxyMap[1]->proxy), "proxy1") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy2", NULL);
    assert(strcmp(storage[0]->model.proxyMap[2]->key, "proxy2") == 0);
    assert(strcmp(storage[0]->model.proxyMap[2]->proxy->getName(storage[0]->model.proxyMap[2]->proxy), "proxy2") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy3", NULL);
    assert(strcmp(storage[0]->model.proxyMap[3]->key, "proxy3") == 0);
    assert(strcmp(storage[0]->model.proxyMap[3]->proxy->getName(storage[0]->model.proxyMap[3]->proxy), "proxy3") == 0);

    // Remove the second proxy (middle) and verify that remaining mediators 0, 2, 3 are shifted correctly
    struct IProxy *removedProxy1 = &(struct Proxy){0}.base;
    assert(model->removeProxy(model, removedProxy1, "proxy1") == true);
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "proxy0") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "proxy0") == 0);
    assert(strcmp(storage[0]->model.proxyMap[1]->key, "proxy2") == 0);
    assert(strcmp(storage[0]->model.proxyMap[1]->proxy->getName(storage[0]->model.proxyMap[1]->proxy), "proxy2") == 0);
    assert(strcmp(storage[0]->model.proxyMap[2]->key, "proxy3") == 0);
    assert(strcmp(storage[0]->model.proxyMap[2]->proxy->getName(storage[0]->model.proxyMap[2]->proxy), "proxy3") == 0);

    // Remove the last proxy and verify the remaining 0, 2 stay in place
    struct IProxy *removedProxy3 = &(struct Proxy){0}.base;
    assert(model->removeProxy(model, removedProxy3, "proxy3") == true);;
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "proxy0") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "proxy0") == 0);
    assert(strcmp(storage[0]->model.proxyMap[1]->key, "proxy2") == 0);
    assert(strcmp(storage[0]->model.proxyMap[1]->proxy->getName(storage[0]->model.proxyMap[1]->proxy), "proxy2") == 0);

    // Remove the first mediator and verify that subsequent mediator 2 shift left
    struct IProxy *removedProxy0 = &(struct Proxy){0}.base;
    assert(model->removeProxy(model, removedProxy0, "proxy0") == true);;
    assert(strcmp(storage[0]->model.proxyMap[0]->key, "proxy2") == 0);
    assert(strcmp(storage[0]->model.proxyMap[0]->proxy->getName(storage[0]->model.proxyMap[0]->proxy), "proxy2") == 0);

    // Remove the remaining mediator3 and confirm that the dictionary is NULL
    struct IProxy *removedProxy2 = &(struct Proxy){0}.base;
    assert(model->removeProxy(model, removedProxy2, "proxy2") == true);;
    assert(storage[0]->model.proxyMap[0]->key[0] == '\0'); // proxyMap is empty
    assert(storage[0]->model.proxyMap[1]->key[0] == '\0');
    assert(storage[0]->model.proxyMap[2]->key[0] == '\0');
    assert(storage[0]->model.proxyMap[3]->key[0] == '\0');

    puremvc_model_removeModel(storage, "ModelTestKey10");
    model = NULL;
}

void TestModelShiftLeft() {
    struct ModelMap *storage[] = { // empty key model storage with 4 slots
        &(struct ModelMap) { .model = {} },
        &(struct ModelMap) { .model = {} },
        &(struct ModelMap) { .model = {} },
        &(struct ModelMap) { .model = {} },
        NULL
    };

    // create 4 instances
    puremvc_model_getInstance(storage, "model0");
    assert(strcmp(storage[0]->key, "model0") == 0);
    assert(strcmp(storage[0]->model.multitonKey, "model0") == 0);

    puremvc_model_getInstance(storage, "model1");
    assert(strcmp(storage[1]->key, "model1") == 0);
    assert(strcmp(storage[1]->model.multitonKey, "model1") == 0);

    puremvc_model_getInstance(storage, "model2");
    assert(strcmp(storage[2]->key, "model2") == 0);
    assert(strcmp(storage[2]->model.multitonKey, "model2") == 0);

    puremvc_model_getInstance(storage, "model3");
    assert(strcmp(storage[3]->key, "model3") == 0);
    assert(strcmp(storage[3]->model.multitonKey, "model3") == 0);

    // remove
    puremvc_model_removeModel(storage, "model1"); // remove middle, remaining 0, 2, 3
    assert(strcmp(storage[0]->key, "model0") == 0);
    assert(strcmp(storage[0]->model.multitonKey, "model0") == 0);
    assert(strcmp(storage[1]->key, "model2") == 0);
    assert(strcmp(storage[1]->model.multitonKey, "model2") == 0);
    assert(strcmp(storage[2]->key, "model3") == 0);
    assert(strcmp(storage[2]->model.multitonKey, "model3") == 0);
    assert(storage[3]->key[0] == '\0');

    puremvc_model_removeModel(storage, "model3"); // remove last, remaining 0, 2
    assert(strcmp(storage[0]->key, "model0") == 0);
    assert(strcmp(storage[0]->model.multitonKey, "model0") == 0);
    assert(strcmp(storage[1]->key, "model2") == 0);
    assert(strcmp(storage[1]->model.multitonKey, "model2") == 0);
    assert(storage[2]->key[0] == '\0');
    assert(storage[3]->key[0] == '\0');

    puremvc_model_removeModel(storage, "model0"); // remove first, remaining 2
    assert(strcmp(storage[0]->key, "model2") == 0);
    assert(strcmp(storage[0]->model.multitonKey, "model2") == 0);
    assert(storage[1]->key[0] == '\0');
    assert(storage[2]->key[0] == '\0');
    assert(storage[3]->key[0] == '\0');

    puremvc_model_removeModel(storage, "model2"); // remove remaining
    assert(storage[0]->key[0] == '\0');
    assert(storage[1]->key[0] == '\0');
    assert(storage[2]->key[0] == '\0');
    assert(storage[3]->key[0] == '\0');
}
