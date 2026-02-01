#include <assert.h>
#include <string.h>

#include "model_test.h"
#include "puremvc/model.h"

#include <stdio.h>

#include "model_test_proxy.h"

int main() {
    testGetInstance();
    testRegisterAndRetrieveProxy();
    testRegisterAndRemoveProxy();
    testHasProxy();
    testOnRegisterAndOnRemove();
    testRemoveModel();
    testMultipleModels();
    testRegisterAndReplaceProxy();
    testRegisterAndRemoveMultipleProxies();
    testGetAndRemoveMultipleInstances();
    testCapacityWarning();
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct Model *model = puremvc_model_getInstance("ModelTestKey1", puremvc_model);
    assert(model != NULL);

    // test assertions
    assert(model == puremvc_model_getInstance("ModelTestKey1", puremvc_model));
    puremvc_model_removeModel("ModelTestKey1");
    model = NULL;
}

void testRegisterAndRetrieveProxy() {
    // register a new and retrieve it.
    struct Model *model = puremvc_model_getInstance("ModelTestKey2", puremvc_model);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    model->registerProxy(model, puremvc_proxy("colors", colors));

    const struct Proxy *proxy = model->retrieveProxy(model, "colors");
    assert(proxy != NULL);
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    const struct Proxy removedProxy = model->removeProxy(model, "colors");
    assert(strcmp(removedProxy.getName(&removedProxy), "colors") == 0);

    assert(model->retrieveProxy(model, "colors") == NULL);
    puremvc_model_removeModel("ModelTestKey2");
    model = NULL;
}

void testRegisterAndRemoveProxy() {
    // register a new, remove it, then try to retrieve it
    struct Model *model = puremvc_model_getInstance("ModelTestKey4", puremvc_model);

    int *sizes = (int []) {1, 2, 3, 0};
    struct Proxy p = puremvc_proxy("sizes", sizes);
    model->registerProxy(model, p);

    // remove the new
    struct Proxy removedProxy = model->removeProxy(model, "sizes");

    // assert that we removed the appropriate new
    assert(strcmp(removedProxy.getName(&removedProxy), "sizes") == 0);

    // ensure that the new is no longer retrievable from the model
    assert(model->retrieveProxy(model, "sizes") == NULL);

    puremvc_model_removeModel("ModelTestKey4");
    model = NULL;
}

void testHasProxy() {
    // register a new
    struct Model *model = puremvc_model_getInstance("ModelTestKey5", puremvc_model);

    const char **aces = (const char *[]) {"clubs", "spades", "hearts", "diamonds", NULL};
    struct Proxy p = puremvc_proxy("aces", aces);
    model->registerProxy(model, p);

    // assert that the model.hasProxy method returns true
    // for that new name
    assert(model->hasProxy(model, "aces") == true);

    // remove the new
    const struct Proxy proxy = model->removeProxy(model, "aces");
    assert(strcmp(proxy.getName(&proxy), "aces") == 0);

    // assert that the model.hasProxy method returns false
    // for that new name
    assert(model->hasProxy(model, "aces") == false);

    puremvc_model_removeModel("ModelTestKey5");
    model = NULL;
}

void testOnRegisterAndOnRemove() {
    // Get a Multiton Model instance
    struct Model *model = puremvc_model_getInstance("ModelTestKey6", puremvc_model);

    // Create and register the test proxy
    model->registerProxy(model, model_test_proxy("ModelTestProxy", NULL));

    // assert that onRegister was called, and the new responded by setting its data accordingly
    const struct Proxy *proxy = model->retrieveProxy(model, "ModelTestProxy");
    assert(strcmp(proxy->getData(proxy), ON_REGISTER_CALLED) == 0);

    // Remove the proxy
    const struct Proxy removedProxy = model->removeProxy(model, "ModelTestProxy");
    assert(strcmp(removedProxy.getName(&removedProxy), "ModelTestProxy") == 0);

    // assert that onRemove was called, and the new responded by setting its data accordingly
    assert(strcmp(removedProxy.getData(&removedProxy), ON_REMOVE_CALLED) == 0);

    puremvc_model_removeModel("ModelTestKey6");
    model = NULL;
}

void testRemoveModel() {
    // Get a Multiton Model instance
    puremvc_model_getInstance("ModelTestKey6", puremvc_model);

    // remove the model
    puremvc_model_removeModel("ModelTestKey6");

    // re-create the model without throwing an exception
    puremvc_model("ModelTestKey6");

    // cleanup
    puremvc_model_removeModel("ModelTestKey6");
}

void testMultipleModels() {
    // Get a Multiton Model instance
    struct Model *model1 = puremvc_model_getInstance("ModelTestKey7", puremvc_model);
    struct Model *model2 = puremvc_model_getInstance("ModelTestKey8", puremvc_model);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    const char **aces = (const char *[]) { "clubs", "spades", "hearts", "diamonds", NULL};

    struct Proxy p1 = puremvc_proxy("colors", colors);
    model1->registerProxy(model1, p1);
    struct Proxy p2 = puremvc_proxy("aces", aces);
    model2->registerProxy(model2, p2);

    assert(model1->hasProxy(model1, "colors"));
    assert(!model2->hasProxy(model2, "colors"));

    assert(!model1->hasProxy(model1, "aces"));
    assert(model2->hasProxy(model2, "aces"));

    struct Proxy proxy = model1->removeProxy(model1, "colors");
    assert(strcmp(proxy.getName(&proxy), "colors") == 0);
    struct Proxy removedProxy = model2->removeProxy(model2, "aces");
    assert(strcmp(removedProxy.getName(&removedProxy), "aces") == 0);

    puremvc_model_removeModel("ModelTestKey7");
    puremvc_model_removeModel("ModelTestKey8");
}

void testRegisterAndReplaceProxy() {
    struct Model *model = puremvc_model_getInstance("ModelTestKey9", puremvc_model);

    int *sizes = (int []) {1, 0};
    struct Proxy p1 = puremvc_proxy("sizes", sizes);
    model->registerProxy(model, p1);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    struct Proxy p2 = puremvc_proxy("sizes", colors);
    model->registerProxy(model, p2);

    const struct Proxy *proxy = model->retrieveProxy(model, "sizes");

    assert(proxy != NULL);
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    const struct Proxy removedProxy = model->removeProxy(model, "sizes");
    assert(strcmp(removedProxy.getName(&removedProxy), "sizes") == 0);

    assert(model->retrieveProxy(model, "sizes") == NULL);
    puremvc_model_removeModel("ModelTestKey9");
    model = NULL;
}

void testRegisterAndRemoveMultipleProxies() {
    struct Model *model = puremvc_model_getInstance("ModelTestKey10", puremvc_model);

    // Register five proxies and verify that each is correctly associated to their dictionaries
    model->registerProxy(model, puremvc_proxy("proxy1", NULL));
    assert(strcmp(model->proxyMap[0].key, "proxy1") == 0);
    assert(strcmp(model->proxyMap[0].proxy.name, "proxy1") == 0);

    model->registerProxy(model, puremvc_proxy("proxy2", NULL));
    assert(strcmp(model->proxyMap[1].key, "proxy2") == 0);
    assert(strcmp(model->proxyMap[1].proxy.name, "proxy2") == 0);

    model->registerProxy(model, puremvc_proxy("proxy3", NULL));
    assert(strcmp(model->proxyMap[2].key, "proxy3") == 0);
    assert(strcmp(model->proxyMap[2].proxy.name, "proxy3") == 0);

    model->registerProxy(model, puremvc_proxy("proxy4", NULL));
    assert(strcmp(model->proxyMap[3].key, "proxy4") == 0);
    assert(strcmp(model->proxyMap[3].proxy.name, "proxy4") == 0);

    model->registerProxy(model, puremvc_proxy("proxy5", NULL));
    assert(strcmp(model->proxyMap[4].key, "proxy5") == 0);
    assert(strcmp(model->proxyMap[4].proxy.name, "proxy5") == 0);

    // Remove the second proxy (middle) and verify that remaining mediators 3, 4, 5 are shifted correctly
    model->removeProxy(model, "proxy2");
    assert(strcmp(model->proxyMap[0].key, "proxy1") == 0);
    assert(strcmp(model->proxyMap[0].proxy.name, "proxy1") == 0);
    assert(strcmp(model->proxyMap[1].key, "proxy3") == 0);
    assert(strcmp(model->proxyMap[1].proxy.name, "proxy3") == 0);
    assert(strcmp(model->proxyMap[2].key, "proxy4") == 0);
    assert(strcmp(model->proxyMap[2].proxy.name, "proxy4") == 0);
    assert(strcmp(model->proxyMap[3].key, "proxy5") == 0);
    assert(strcmp(model->proxyMap[3].proxy.name, "proxy5") == 0);

    // Remove the last proxy and verify the remaining ones stay in place
    model->removeProxy(model, "proxy5");
    assert(strcmp(model->proxyMap[0].key, "proxy1") == 0);
    assert(strcmp(model->proxyMap[0].proxy.name, "proxy1") == 0);
    assert(strcmp(model->proxyMap[1].key, "proxy3") == 0);
    assert(strcmp(model->proxyMap[1].proxy.name, "proxy3") == 0);
    assert(strcmp(model->proxyMap[2].key, "proxy4") == 0);
    assert(strcmp(model->proxyMap[2].proxy.name, "proxy4") == 0);

    // Remove the first mediator and verify that subsequent mediators 3, 4 shift left
    model->removeProxy(model, "proxy1");
    assert(strcmp(model->proxyMap[0].key, "proxy3") == 0);
    assert(strcmp(model->proxyMap[0].proxy.name, "proxy3") == 0);
    assert(strcmp(model->proxyMap[1].key, "proxy4") == 0);
    assert(strcmp(model->proxyMap[1].proxy.name, "proxy4") == 0);

    // Remove all remaining mediators and confirm that the dictionary is NULL
    model->removeProxy(model, "proxy3");
    model->removeProxy(model, "proxy4");
    assert(model->proxyMap->key[0] == '\0'); // proxyMap is empty

    puremvc_model_removeModel("ModelTestKey10");
    model = NULL;
}

void testGetAndRemoveMultipleInstances() {
    puremvc_model_getInstance("model1", puremvc_model);
    puremvc_model_getInstance("model2", puremvc_model);
    puremvc_model_getInstance("model3", puremvc_model);
    puremvc_model_getInstance("model4", puremvc_model);

    puremvc_model_removeModel("model2"); // remove middle
    puremvc_model_removeModel("model4"); // remove last
    puremvc_model_removeModel("model1"); // remove first
    puremvc_model_removeModel("model3"); // remove remaining
}

void testCapacityWarning() {
    for (int i = 0; i < INSTANCE_MAP_SIZE + 1; i++) {
        char key[32] = {0};
        snprintf(key, sizeof(key), "model%d", i);
        puremvc_model_getInstance(key, puremvc_model);
    }

    struct Model *model = puremvc_model_getInstance("model1", puremvc_model);
    for (int i = 0; i < PROXY_MAP_SIZE + 1; i++) {
        char key[32] = {0};
        snprintf(key, sizeof(key), "proxy%d", i);
        model->registerProxy(model, puremvc_proxy(key, NULL));
    }

    for (int i = 0; i < INSTANCE_MAP_SIZE + 1; i++) {
        char key[32] = {0};
        snprintf(key, sizeof(key), "model%d", i);
        puremvc_model_removeModel(key);
    }
}
