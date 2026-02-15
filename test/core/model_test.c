#include "model_test.h"

#include "puremvc/platform.h"
#include "puremvc/i_model.h"
#include "model_test_proxy.h"

#include <stdio.h>
#include <string.h>
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

int main() {
    printf("\n\033[1;36m================================================\033[0m\n");
    printf("\033[1;36m[SUITE] %s\033[0m\n", "ModelTest");
    printf("\033[1;36m================================================\033[0m\n\n");

    beforeAll();
    test("testGetInstance", testGetInstance);
    test("testRegisterAndRetrieveProxy", testRegisterAndRetrieveProxy);
    test("testHasProxy", testHasProxy);
    test("testRegisterAndRemoveProxy", testRegisterAndRemoveProxy);
    test("testOnRegisterAndOnRemove", testOnRegisterAndOnRemove);
    test("testRemoveModel", testRemoveModel);
    test("testRegisterAndReplaceProxy", testRegisterAndReplaceProxy);
    test("testProxyMapShiftLeft", testProxyMapShiftLeft);
    test("TestModelMapShiftLeft", TestModelMapShiftLeft);
    afterAll();

    printf("\n\033[1;32m[DONE] All tests in suite finished.\033[0m\n");
    return 0;
}

void testGetInstance() {
    struct ModelMap **instanceMap = (struct ModelMap *[]){
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    // Test Factory Method
    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey1");
    model->initializeModel(model, NULL);

    // test assertions
    if (model == NULL) abort();
    if (model != puremvc_model_getInstance(instanceMap, "ModelTestKey1")) abort();
    if (strcmp(instanceMap[0]->key, "ModelTestKey1") != 0) abort();

    struct IModel *removedModel = NULL;
    if (puremvc_model_removeModel("ModelTestKey1", &removedModel) != true) abort();
    if (instanceMap[0]->key[0] != '\0') abort();
}

void testRegisterAndRetrieveProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        NULL
    };

    // alignas(ALIGNMENT) unsigned char buffer[puremvc_proxy_size()];
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        NULL
    };

    // register a proxy and retrieve it.
    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey2");
    model->initializeModel(model, proxyMap);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    model->registerProxy(model, puremvc_proxy_init, "colors", colors);

    // Retrieve the proxy
    const struct IProxy *proxy = model->retrieveProxy(model, "colors");

    // test assertions
    if (proxy == NULL) abort();
    if (proxy->getData(proxy) != colors) abort();
    const char **data = proxy->getData(proxy);
    if (strcmp(*data, "red") != 0) abort();
    if (strcmp(*(data + 1), "green") != 0) abort();
    if (strcmp(*(data + 2), "blue") != 0) abort();

    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "colors", &removedProxy);
    if (strcmp(removedProxy->getName(removedProxy), "colors") != 0) abort();

    model->retrieveProxy(model, "colors");

    if (puremvc_model_removeModel("ModelTestKey2", NULL) != true) abort();
    model = NULL;
}

void testHasProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    alignas(ALIGNMENT) unsigned char buffer[puremvc_proxy_size()];
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = (struct IProxy *) buffer }, // (struct IProxy *) buffer
        NULL
    };

    // Get the Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey3");
    model->initializeModel(model, proxyMap);

    // Create and register the test mediator
    const char **aces = (const char *[]) {"clubs", "spades", "hearts", "diamonds", NULL};
    model->registerProxy(model, puremvc_proxy_init, "aces", aces);

    // assert that the model.hasProxy method returns true
    // for that proxy name
    if (model->hasProxy(model, "aces") != true) abort();

    // remove the proxy
    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "aces", &removedProxy);
    if (strcmp(removedProxy->getName(removedProxy), "aces") != 0) abort();

    // assert that the model->hasProxy method returns false for that proxy name
    if (model->hasProxy(model, "aces") != false) abort();

    if (puremvc_model_removeModel("ModelTestKey3", NULL) != true) abort();
}

void testRegisterAndRemoveProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    // alignas(ALIGNMENT) unsigned char buffer[puremvc_proxy_size()];
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        NULL
    };

    // Get the Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey4");
    model->initializeModel(model, proxyMap);

    int *sizes = (int []) {1, 2, 3, 0};

    // Register the proxy
    model->registerProxy(model, puremvc_proxy_init, "sizes", sizes);

    // remove the proxy
    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "sizes", &removedProxy);

    // assert that we removed the appropriate proxy
    if (strcmp(removedProxy->getName(removedProxy), "sizes") != 0) abort();

    // ensure that the proxy is no longer retrievable from the model
    if (model->retrieveProxy(model, "sizes") != NULL) abort();

    if (puremvc_model_removeModel("ModelTestKey4", NULL) != true) abort();
    model = NULL;
}

void testOnRegisterAndOnRemove() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    // alignas(ALIGNMENT) unsigned char buffer[puremvc_proxy_size()];
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        NULL
    };

    // Get a Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey5");
    model->initializeModel(model, proxyMap);

    // Create and register the test proxy
    model->registerProxy(model, model_test_proxy, "ModelTestProxy", NULL);

    // assert that onRegister was called, and the proxy responded by setting its data accordingly
    const struct IProxy *proxy = model->retrieveProxy(model, "ModelTestProxy");
    if (strcmp(proxy->getData(proxy), ON_REGISTER_CALLED) != 0) abort();

    // Remove the proxy
    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "ModelTestProxy", &removedProxy);
    if (strcmp(removedProxy->getName(removedProxy), "ModelTestProxy") != 0) abort();

    // assert that onRemove was called, and the proxy responded by setting its data accordingly
    if (strcmp(removedProxy->getData(removedProxy), ON_REMOVE_CALLED) != 0) abort();

    if (puremvc_model_removeModel("ModelTestKey5", NULL) != true) abort();
    model = NULL;
}

void testRemoveModel() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    // Get a Multiton Model instance
    puremvc_model_getInstance(instanceMap, "ModelTestKey6");

    if (strcmp(instanceMap[0]->key, "ModelTestKey6") != 0) abort();

    // remove the model
    if (puremvc_model_removeModel("ModelTestKey6", NULL) != true) abort();
    if (instanceMap[0]->key[0] != '\0') abort();

    // re-create the model without throwing an exception
    puremvc_model_getInstance(instanceMap, "ModelTestKey6");
    if (strcmp(instanceMap[0]->key, "ModelTestKey6") != 0) abort();

    // try removing again
    if (puremvc_model_removeModel("ModelTestKey6", NULL) != true) abort();;
    if (instanceMap[0]->key[0] != '\0') abort();
}

void testRegisterAndReplaceProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    // alignas(ALIGNMENT) unsigned char buffer[puremvc_proxy_size()];
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        NULL
    };

    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey9");
    model->initializeModel(model, proxyMap);

    int *sizes = (int []) {1, 0};
    if (model->registerProxy(model, puremvc_proxy_init, "sizes", sizes) != true) abort();

    // replace with another proxy
    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    // static const char *colors[] = {"red", "green", "blue", NULL};
    if (model->registerProxy(model, puremvc_proxy_init, "sizes", colors) != true) abort();

    // try to retrieve the replaced proxy
    const struct IProxy *proxy = model->retrieveProxy(model, "sizes");

    // test assertions
    if (proxy == NULL) abort();
    if (strcmp(proxy->getName(proxy), "sizes") != 0) abort();
    struct INotifier *notifier = proxy->getNotifier(proxy);
    if (notifier == NULL) abort();
    if (notifier->getMultitonKey(notifier) == NULL) abort();


    const char **data = proxy->getData(proxy);
    if (data != colors) abort();
    if (strcmp(*data, "red") != 0) abort();
    if (strcmp(*(data + 1), "green") != 0) abort();
    if (strcmp(*(data + 2), "blue") != 0) abort();

    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "sizes", &removedProxy);
    if (strcmp(removedProxy->getName(removedProxy), "sizes") != 0) abort();

    if (model->retrieveProxy(model, "sizes") != NULL) abort();
    if (puremvc_model_removeModel("ModelTestKey9", NULL) != true) abort();
}

void testProxyMapShiftLeft() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size() )},
        NULL
    };

    // alignas(ALIGNMENT) unsigned char buffer0[puremvc_proxy_size()];
    // alignas(ALIGNMENT) unsigned char buffer1[puremvc_proxy_size()];
    // alignas(ALIGNMENT) unsigned char buffer2[puremvc_proxy_size()];
    // alignas(ALIGNMENT) unsigned char buffer3[puremvc_proxy_size()];
    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        NULL
    };

    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey10");
    model->initializeModel(model, proxyMap);

    size_t offset = sizeof(struct IModel) + KEY_SIZE;  // skip base + multitonKey
    struct ProxyMap ***ppp = (struct ProxyMap ***)((char *)model + offset);
    struct ProxyMap **actualMap = *ppp;

    // Register four proxies and verify that each is correctly associated to their dictionaries
    if (model->registerProxy(model, puremvc_proxy_init, "proxy0", NULL) != true) abort();
    const struct IProxy *proxy0 = actualMap[0]->proxy;
    if (strcmp(actualMap[0]->key, "proxy0") != 0) abort();
    if (strcmp(proxy0->getName(proxy0), "proxy0") != 0) abort();

    if (model->registerProxy(model, puremvc_proxy_init, "proxy1", NULL) != true) abort();
    const struct IProxy *proxy1 = actualMap[1]->proxy;
    if (strcmp(actualMap[1]->key, "proxy1") != 0) abort();
    if (strcmp(proxy1->getName(proxy1), "proxy1") != 0) abort();

    if (model->registerProxy(model, puremvc_proxy_init, "proxy2", NULL) != true) abort();
    const struct IProxy *proxy2 = actualMap[2]->proxy;
    if (strcmp(actualMap[2]->key, "proxy2") != 0) abort();
    if (strcmp(proxy2->getName(proxy2), "proxy2") != 0) abort();

    if (model->registerProxy(model, puremvc_proxy_init, "proxy3", NULL) != true) abort();
    const struct IProxy *proxy3 = actualMap[3]->proxy;
    if (strcmp(actualMap[3]->key, "proxy3") != 0) abort();
    if (strcmp(proxy3->getName(proxy3), "proxy3") != 0) abort();

    // Remove the second proxy (middle) and verify that remaining proxies 0, 2, 3 are shifted correctly
    struct IProxy *removedProxy1 = NULL;
    if (model->removeProxy(model, "proxy1", &removedProxy1) != true) abort();
    if (strcmp(removedProxy1->getName(removedProxy1), "proxy1") != 0) abort();
    if (strcmp(actualMap[0]->key, "proxy0") != 0) abort();
    if (strcmp(actualMap[1]->key, "proxy2") != 0) abort();
    if (strcmp(actualMap[2]->key, "proxy3") != 0) abort();

    // Remove the last proxy and verify the remaining 0, 2 stay in place
    struct IProxy *removedProxy3 = NULL;
    if (model->removeProxy(model, "proxy3", &removedProxy3) != true) abort();;
    if (strcmp(removedProxy3->getName(removedProxy3), "proxy3") != 0) abort();
    if (strcmp(actualMap[0]->key, "proxy0") != 0) abort();
    if (strcmp(actualMap[1]->key, "proxy2") != 0) abort();

    // Remove the first proxy and verify that subsequent proxy 2 shift left
    struct IProxy *removedProxy0 = NULL;
    if (model->removeProxy(model, "proxy0", &removedProxy0) != true) abort();;
    if (strcmp(removedProxy0->getName(removedProxy0), "proxy0") != 0) abort();
    if (strcmp(actualMap[0]->key, "proxy2") != 0) abort();

    // Remove the remaining proxy3 and confirm that the dictionary is NULL
    struct IProxy *removedProxy2 = NULL;
    if (model->removeProxy(model, "proxy2", &removedProxy2) != true) abort();;
    if (strcmp(removedProxy2->getName(removedProxy2), "proxy2") != 0) abort();
    if (actualMap[0]->key[0] != '\0') abort(); // proxyMap is empty
    if (actualMap[1]->key[0] != '\0') abort();
    if (actualMap[2]->key[0] != '\0') abort();
    if (actualMap[3]->key[0] != '\0') abort();

    if (puremvc_model_removeModel("ModelTestKey10", NULL) != true) abort();;
    model = NULL;
}

void TestModelMapShiftLeft() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) { // modelMap with 4 slots for the instance
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        NULL
    };

    // create 4 instances
    if (puremvc_model_getInstance(instanceMap, "model0") == NULL) abort();
    if (strcmp(instanceMap[0]->key, "model0") != 0) abort();
    const char *key0 = (char *)instanceMap[0]->model + sizeof(struct IModel);
    if (strcmp(key0, "model0") != 0) abort();

    if (puremvc_model_getInstance(instanceMap, "model1") == NULL) abort();
    if (strcmp(instanceMap[1]->key, "model1") != 0) abort();
    const char *key1 = (char *)instanceMap[1]->model + sizeof(struct IModel);
    if (strcmp(key1, "model1") != 0) abort();

    if (puremvc_model_getInstance(instanceMap, "model2") == NULL) abort();
    if (strcmp(instanceMap[2]->key, "model2") != 0) abort();
    const char *key2 = (char *)instanceMap[2]->model + sizeof(struct IModel);
    if (strcmp(key2, "model2") != 0) abort();

    if (puremvc_model_getInstance(instanceMap, "model3") == NULL) abort();
    if (strcmp(instanceMap[3]->key, "model3") != 0) abort();
    const char *key3 = (char *)instanceMap[3]->model + sizeof(struct IModel);
    if (strcmp(key3, "model3") != 0) abort();

    // remove
    struct IModel *model1 = NULL; // remove middle1, remaining 0, 2, 3
    if (puremvc_model_removeModel("model1", &model1) != true) abort();
    if (strcmp(instanceMap[0]->key, "model0") != 0) abort();
    if (strcmp(instanceMap[1]->key, "model2") != 0) abort();
    if (strcmp(instanceMap[2]->key, "model3") != 0) abort();
    if (instanceMap[3]->key[0] != '\0') abort();
    if (instanceMap[4] != NULL) abort();

    struct IModel *model3 = NULL; // remove last3, remaining 0, 2
    if (puremvc_model_removeModel("model3", &model3) != true) abort();
    if (strcmp(instanceMap[0]->key, "model0") != 0) abort();
    if (strcmp(instanceMap[1]->key, "model2") != 0) abort();
    if (instanceMap[2]->key[0] != '\0') abort();
    if (instanceMap[3]->key[0] != '\0') abort();
    if (instanceMap[4] != NULL) abort();

    struct IModel *model0 = NULL; // remove first, remaining 2
    if (puremvc_model_removeModel("model0", &model0) != true) abort();
    if (strcmp(instanceMap[0]->key, "model2") != 0) abort();
    if (instanceMap[1]->key[0] != '\0') abort();
    if (instanceMap[2]->key[0] != '\0') abort();
    if (instanceMap[3]->key[0] != '\0') abort();
    if (instanceMap[4] != NULL) abort();

    struct IModel *model2 = NULL; // remove remaining
    if (puremvc_model_removeModel("model2", &model2) != true) abort();
    if (instanceMap[0]->key[0] != '\0') abort();
    if (instanceMap[1]->key[0] != '\0') abort();
    if (instanceMap[2]->key[0] != '\0') abort();
    if (instanceMap[3]->key[0] != '\0') abort();
}
