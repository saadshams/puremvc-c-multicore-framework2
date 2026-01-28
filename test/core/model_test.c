#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "model_test.h"
#include "puremvc/model.h"
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
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct Model *model = puremvc_model_getInstance("ModelTestKey1", puremvc_model);
    assert(model != NULL);
    model->initializeModel(model);

    // test assertions
    assert(model == puremvc_model_getInstance("ModelTestKey1", puremvc_model));
    puremvc_model_removeModel("ModelTestKey1");
    model = NULL;
}

void testRegisterAndRetrieveProxy() {
    // register a new and retrieve it.
    struct Model *model = puremvc_model_getInstance("ModelTestKey2", puremvc_model);
    model->initializeModel(model);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    struct Proxy p = puremvc_proxy("colors", colors);
    model->registerProxy(model, &p);

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
    model->initializeModel(model);

    int *sizes = (int []) {1, 2, 3, 0};
    struct Proxy p = puremvc_proxy("sizes", sizes);
    model->registerProxy(model, &p);

    // remove the new
    const struct Proxy removedProxy = model->removeProxy(model, "sizes");

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
    model->initializeModel(model);

    const char **aces = (const char *[]) {"clubs", "spades", "hearts", "diamonds", NULL};
    struct Proxy p = puremvc_proxy("aces", aces);
    model->registerProxy(model, &p);

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
    model->initializeModel(model);

    // Create and register the test proxy
    struct Proxy proxy = model_test_proxy("ModelTestProxy", NULL);
    model->registerProxy(model, &proxy);

    // assert that onRegister was called, and the new responded by setting its data accordingly
    assert(strcmp(proxy.getData(&proxy), ON_REGISTER_CALLED) == 0);

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
    struct Model *model = puremvc_model_getInstance("ModelTestKey6", puremvc_model);
    model->initializeModel(model);

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
    model1->initializeModel(model1);
    struct Model *model2 = puremvc_model_getInstance("ModelTestKey8", puremvc_model);
    model2->initializeModel(model2);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    const char **aces = (const char *[]) { "clubs", "spades", "hearts", "diamonds", NULL};

    struct Proxy p1 = puremvc_proxy("colors", colors);
    model1->registerProxy(model1, &p1);
    struct Proxy p2 = puremvc_proxy("aces", aces);
    model2->registerProxy(model2, &p2);

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
    struct Model *model = puremvc_model_getInstance("ModelTestKey8", puremvc_model);
    model->initializeModel(model);

    int *sizes = (int []) {1, 0};
    struct Proxy p1 = puremvc_proxy("sizes", sizes);
    model->registerProxy(model, &p1);

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    struct Proxy p2 = puremvc_proxy("sizes", colors);
    model->registerProxy(model, &p2);

    const struct Proxy *proxy = model->retrieveProxy(model, "sizes");

    assert(proxy != NULL);
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    struct Proxy removedProxy = model->removeProxy(model, "sizes");
    assert(strcmp(removedProxy.getName(&removedProxy), "sizes") == 0);

    assert(model->retrieveProxy(model, "sizes") == NULL);
    puremvc_model_removeModel("ModelTestKey8");
    model = NULL;
}
