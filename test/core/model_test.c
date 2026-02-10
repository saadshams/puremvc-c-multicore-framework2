#include "model_test.h"

#include "puremvc/i_model.h"
#include "model_test_proxy.h"

#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    testGetInstance();
    testRegisterAndRetrieveProxy();
    testHasProxy();
    testRegisterAndRemoveProxy();
    testOnRegisterAndOnRemove();
    testRemoveModel();
    testRegisterAndReplaceProxy();
    testProxyMapShiftLeft();
    TestModelShiftLeft();
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
    assert(model != NULL);
    assert(model == puremvc_model_getInstance(instanceMap, "ModelTestKey1"));
    assert(strcmp(instanceMap[0]->key, "ModelTestKey1") == 0);

    struct IModel *removedModel = NULL;
    assert(puremvc_model_removeModel("ModelTestKey1", &removedModel) == true);
    assert(instanceMap[0]->key == NULL);
}

void testRegisterAndRetrieveProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        NULL
    };

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
    assert(proxy != NULL);
    assert(proxy->getData(proxy) == colors);
    const char **data = proxy->getData(proxy);
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "colors", &removedProxy);
    assert(strcmp(removedProxy->getName(removedProxy), "colors") == 0);

    model->retrieveProxy(model, "colors");

    assert(puremvc_model_removeModel("ModelTestKey2", NULL) == true);
    model = NULL;
}

void testHasProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size() )},
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
    assert(model->hasProxy(model, "aces") == true);

    // remove the proxy
    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "aces", &removedProxy);
    assert(strcmp(removedProxy->getName(removedProxy), "aces") == 0);

    // assert that the model->hasProxy method returns false for that proxy name
    assert(model->hasProxy(model, "aces") == false);

    assert(puremvc_model_removeModel("ModelTestKey3", NULL) == true);
}

void testRegisterAndRemoveProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size() )},
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
    assert(strcmp(removedProxy->getName(removedProxy), "sizes") == 0);

    // ensure that the proxy is no longer retrievable from the model
    assert(model->retrieveProxy(model, "sizes") == NULL);

    assert(puremvc_model_removeModel("ModelTestKey4", NULL) == true);
    model = NULL;
}

void testOnRegisterAndOnRemove() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size() )},
        NULL
    };

    // Get a Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey5");
    model->initializeModel(model, proxyMap);

    // Create and register the test proxy
    model->registerProxy(model, model_test_proxy, "ModelTestProxy", NULL);

    // assert that onRegister was called, and the proxy responded by setting its data accordingly
    const struct IProxy *proxy = model->retrieveProxy(model, "ModelTestProxy");
    assert(strcmp(proxy->getData(proxy), ON_REGISTER_CALLED) == 0);

    // Remove the proxy
    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "ModelTestProxy", &removedProxy);
    assert(strcmp(removedProxy->getName(removedProxy), "ModelTestProxy") == 0);

    // assert that onRemove was called, and the proxy responded by setting its data accordingly
    assert(strcmp(removedProxy->getData(removedProxy), ON_REMOVE_CALLED) == 0);

    assert(puremvc_model_removeModel("ModelTestKey5", NULL) == true);
    model = NULL;
}

void testRemoveModel() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    // Get a Multiton Model instance
    puremvc_model_getInstance(instanceMap, "ModelTestKey6");

    assert(strcmp(instanceMap[0]->key, "ModelTestKey6") == 0);

    // remove the model
    assert(puremvc_model_removeModel("ModelTestKey6", NULL) == true);
    assert(instanceMap[0]->key == NULL);

    // re-create the model without throwing an exception
    puremvc_model_getInstance(instanceMap, "ModelTestKey6");
    assert(strcmp(instanceMap[0]->key, "ModelTestKey6") == 0);

    // try removing again
    assert(puremvc_model_removeModel("ModelTestKey6", NULL) == true);;
    assert(instanceMap[0]->key == NULL);
}

void testRegisterAndReplaceProxy() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size()) },
        NULL
    };

    struct ProxyMap **proxyMap = (struct ProxyMap *[]){
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        NULL
    };

    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey9");
    model->initializeModel(model, proxyMap);

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

    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "sizes", &removedProxy);
    assert(strcmp(removedProxy->getName(removedProxy), "sizes") == 0);

    assert(model->retrieveProxy(model, "sizes") == NULL);
    assert(puremvc_model_removeModel("ModelTestKey9", NULL) == true);
    model = NULL;
}

void testProxyMapShiftLeft() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) {
        &(struct ModelMap){ .model = alloca(puremvc_model_size() )},
        NULL
    };

    struct ProxyMap **proxyMap = (struct ProxyMap *[]) {
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        &(struct ProxyMap){ .proxy = alloca(puremvc_proxy_size()) },
        NULL
    };

    struct IModel *model = puremvc_model_getInstance(instanceMap, "ModelTestKey10");
    model->initializeModel(model, proxyMap);

    size_t offset = sizeof(struct IModel) + sizeof(const char *);
    struct ProxyMap ***ppp = (struct ProxyMap ***)((char *) model + offset);
    struct ProxyMap **actualMap = *ppp;

    // Register four proxies and verify that each is correctly associated to their dictionaries
    model->registerProxy(model, puremvc_proxy_init, "proxy0", NULL);
    const struct IProxy *proxy0 = actualMap[0]->proxy;
    assert(strcmp(actualMap[0]->key, "proxy0") == 0);
    assert(strcmp(proxy0->getName(proxy0), "proxy0") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy1", NULL);
    const struct IProxy *proxy1 = actualMap[1]->proxy;
    assert(strcmp(actualMap[1]->key, "proxy1") == 0);
    assert(strcmp(proxy1->getName(proxy1), "proxy1") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy2", NULL);
    const struct IProxy *proxy2 = actualMap[2]->proxy;
    assert(strcmp(actualMap[2]->key, "proxy2") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy3", NULL);
    const struct IProxy *proxy3 = actualMap[3]->proxy;
    assert(strcmp(actualMap[3]->key, "proxy3") == 0);
    assert(strcmp(proxy3->getName(proxy3), "proxy3") == 0);

    // Remove the second proxy (middle) and verify that remaining proxies 0, 2, 3 are shifted correctly
    struct IProxy *removedProxy1 = NULL;
    assert(model->removeProxy(model, "proxy1", &removedProxy1) == true);
    assert(strcmp(removedProxy1->getName(removedProxy1), "proxy1") == 0);
    assert(strcmp(actualMap[0]->key, "proxy0") == 0);
    assert(strcmp(actualMap[1]->key, "proxy2") == 0);
    assert(strcmp(actualMap[2]->key, "proxy3") == 0);

    // Remove the last proxy and verify the remaining 0, 2 stay in place
    struct IProxy *removedProxy3 = NULL;
    assert(model->removeProxy(model, "proxy3", &removedProxy3) == true);;
    assert(strcmp(removedProxy3->getName(removedProxy3), "proxy3") == 0);
    assert(strcmp(actualMap[0]->key, "proxy0") == 0);
    assert(strcmp(actualMap[1]->key, "proxy2") == 0);

    // Remove the first proxy and verify that subsequent proxy 2 shift left
    struct IProxy *removedProxy0 = NULL;
    assert(model->removeProxy(model, "proxy0", &removedProxy0) == true);;
    assert(strcmp(removedProxy0->getName(removedProxy0), "proxy0") == 0);
    assert(strcmp(actualMap[0]->key, "proxy2") == 0);

    // Remove the remaining proxy3 and confirm that the dictionary is NULL
    struct IProxy *removedProxy2 = NULL;
    assert(model->removeProxy(model, "proxy2", &removedProxy2) == true);;
    assert(strcmp(removedProxy2->getName(removedProxy2), "proxy2") == 0);
    assert(actualMap[0]->key == NULL); // proxyMap is empty
    assert(actualMap[1]->key == NULL);
    assert(actualMap[2]->key == NULL);
    assert(actualMap[3]->key == NULL);

    assert(puremvc_model_removeModel("ModelTestKey10", NULL) == true);;
    model = NULL;
}

void TestModelShiftLeft() {
    struct ModelMap **instanceMap = (struct ModelMap *[]) { // modelMap with 4 slots for the instance
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        &(struct ModelMap) { .model = alloca(puremvc_model_size()) },
        NULL
    };

    // create 4 instances
    puremvc_model_getInstance(instanceMap, "model0");
    assert(strcmp(instanceMap[0]->key, "model0") == 0);
    const char **key0 = (const char **)((char *) instanceMap[0]->model + sizeof(struct IModel));
    assert(strcmp(*key0, "model0") == 0);

    puremvc_model_getInstance(instanceMap, "model1");
    assert(strcmp(instanceMap[1]->key, "model1") == 0);
    const char **key1 = (const char **)((char *) instanceMap[0]->model + sizeof(struct IModel));
    assert(strcmp(*key1, "model0") == 0);

    puremvc_model_getInstance(instanceMap, "model2");
    assert(strcmp(instanceMap[2]->key, "model2") == 0);
    const char **key2 = (const char **)((char *) instanceMap[0]->model + sizeof(struct IModel));
    assert(strcmp(*key2, "model0") == 0);

    puremvc_model_getInstance(instanceMap, "model3");
    assert(strcmp(instanceMap[3]->key, "model3") == 0);
    const char **key3 = (const char **)((char *) instanceMap[0]->model + sizeof(struct IModel));
    assert(strcmp(*key3, "model0") == 0);

    // remove
    struct IModel *removedModel1 = NULL;
    assert(puremvc_model_removeModel("model1", &removedModel1) == true); // remove middle, remaining 0, 2, 3
    key1 = (const char **)((char *) removedModel1 + sizeof(struct IModel)); // multitonKey
    assert(strcmp(*key1, "model1") == 0);
    assert(strcmp(*((const char **)((char *) instanceMap[0]->model + sizeof(struct IModel))), "model0") == 0); // model->multitonKey
    assert(strcmp(*((const char **)((char *) instanceMap[1]->model + sizeof(struct IModel))), "model2") == 0);
    assert(strcmp(*((const char **)((char *) instanceMap[2]->model + sizeof(struct IModel))), "model3") == 0);
    assert(strcmp(instanceMap[0]->key, "model0") == 0); // instanceMap->key
    assert(strcmp(instanceMap[1]->key, "model2") == 0);
    assert(strcmp(instanceMap[2]->key, "model3") == 0);
    assert(instanceMap[3]->key == NULL);

    struct IModel *removedModel3 = NULL;
    assert(puremvc_model_removeModel("model3", &removedModel3) == true); // remove last, remaining 0, 2
    key3 = (const char **)((char *) removedModel3 + sizeof(struct IModel)); // multitonKey
    assert(strcmp(*key3, "model3") == 0);
    assert(strcmp(*((const char **)((char *) instanceMap[0]->model + sizeof(struct IModel))), "model0") == 0);
    assert(strcmp(*((const char **)((char *) instanceMap[1]->model + sizeof(struct IModel))), "model2") == 0);
    assert(strcmp(instanceMap[0]->key, "model0") == 0);
    assert(strcmp(instanceMap[1]->key, "model2") == 0);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);

    struct IModel *removedModel0 = NULL;
    assert(puremvc_model_removeModel("model0", &removedModel0) == true); // remove first, remaining 2
    key0 = (const char **)((char *) removedModel0 + sizeof(struct IModel)); // multitonKey
    assert(strcmp(*key0, "model0") == 0);
    assert(strcmp(*((const char **)((char *) instanceMap[0]->model + sizeof(struct IModel))), "model2") == 0);
    assert(strcmp(instanceMap[0]->key, "model2") == 0);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);

    struct IModel *model2 = NULL;
    assert(puremvc_model_removeModel("model2", &model2) == true); // remove remaining
    assert(instanceMap[0]->key == NULL);
    assert(instanceMap[1]->key == NULL);
    assert(instanceMap[2]->key == NULL);
    assert(instanceMap[3]->key == NULL);
}
