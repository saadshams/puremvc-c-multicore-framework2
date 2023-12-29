#include "model_test.h"
#include "model_test_proxy.h"
#include "../../include/model.h"
#include <assert.h>
#include <string.h>

int main() {
    testGetInstance();
    testRegisterAndRetrieveProxy();
    testRegisterAndRemoveProxy();
    testHasProxy();
    testOnRegisterAndOnRemove();
    testRemoveModel();
    testMultipleModels();
    return 0;
}

void testGetInstance() {
    // Test Factory Method
    struct Model *model = puremvc_model_getInstance("ModelTestKey1", puremvc_model_new);
    assert(model != NULL);

    // test assertions
    assert(model == puremvc_model_getInstance("ModelTestKey1", puremvc_model_new));
    puremvc_model_removeModel("ModelTestKey1");
    model = NULL;
}

void testRegisterAndRetrieveProxy() {
    // register a new and retrieve it.
    struct Model *model = puremvc_model_getInstance("ModelTestKey2", puremvc_model_new);
    model->registerProxy(model, puremvc_proxy_new("colors", (char *[]) {"red", "green", "blue", NULL}));
    struct Proxy *proxy = model->retrieveProxy(model, "colors");

    assert(proxy != NULL);
    const char **data = proxy->getData(proxy);

    // test assertions
    assert(strcmp(*data, "red") == 0);
    assert(strcmp(*(data + 1), "green") == 0);
    assert(strcmp(*(data + 2), "blue") == 0);

    struct Proxy *removedProxy = model->removeProxy(model, "colors");
    assert(strcmp(removedProxy->proxyName, "colors") == 0);
    puremvc_proxy_free(&removedProxy);
    assert(model->retrieveProxy(model, "colors") == NULL);
    puremvc_model_removeModel("ModelTestKey2");
    model = NULL;
}

void testRegisterAndRemoveProxy() {
    // register a new, remove it, then try to retrieve it
    struct Model *model = puremvc_model_getInstance("ModelTestKey4", puremvc_model_new);
    model->registerProxy(model, puremvc_proxy_new("sizes", (char *[]) {"7", "13", "21"}));

    // remove the new
    struct Proxy *removedProxy = model->removeProxy(model, "sizes");

    // assert that we removed the appropriate new
    assert(strcmp(removedProxy->proxyName, "sizes") == 0);

    // ensure that the new is no longer retrievable from the model
    assert(model->retrieveProxy(model, "sizes") == NULL);

    puremvc_proxy_free(&removedProxy);
    puremvc_model_removeModel("ModelTestKey4");
    model = NULL;
}

void testHasProxy() {
    // register a new
    struct Model *model = puremvc_model_getInstance("ModelTestKey5", puremvc_model_new);
    model->registerProxy(model, puremvc_proxy_new("aces", (char *[]) {"clubs", "spades", "hearts", "diamonds"}));

    // assert that the model.hasProxy method returns true
    // for that new name
    assert(model->hasProxy(model, "aces") == true);

    // remove the new
    struct Proxy *proxy = model->removeProxy(model, "aces");

    assert(proxy != NULL);
    puremvc_proxy_free(&proxy);

    // assert that the model.hasProxy method returns false
    // for that new name
    assert(model->hasProxy(model, "aces") == false);

    puremvc_model_removeModel("ModelTestKey5");
    model = NULL;
}

void testOnRegisterAndOnRemove() {
    // Get a Multiton Model instance
    struct Model *model = puremvc_model_getInstance("ModelTestKey6", puremvc_model_new);

    // Create and register the test mediator
    ModelTestProxy *modelTestProxy = model_test_proxy_new("ModelTestProxy", NULL);
    model->registerProxy(model, (struct Proxy *) modelTestProxy);

    // assert that onRegister was called, and the new responded by setting its data accordingly
    assert(strcmp(modelTestProxy->proxy.getData(&modelTestProxy->proxy), ON_REGISTER_CALLED) == 0);

    // Remove the component
    model->removeProxy(model, "ModelTestProxy");

    // assert that onRemove was called, and the new responded by setting its data accordingly
    assert(strcmp(modelTestProxy->proxy.data, ON_REMOVE_CALLED) == 0);
    model_test_proxy_free(&modelTestProxy);

    puremvc_model_removeModel("ModelTestKey6");
    model = NULL;
}

void testRemoveModel() {
    // Get a Multiton Model instance
    puremvc_model_getInstance("ModelTestKey6", puremvc_model_new);

    // remove the model
    puremvc_model_removeModel("ModelTestKey6");

    // re-create the model without throwing an exception
    puremvc_model_new("ModelTestKey6");

    // cleanup
    puremvc_model_removeModel("ModelTestKey6");
}

void testMultipleModels() {
    // Get a Multiton Model instance
    struct Model *model1 = puremvc_model_getInstance("ModelTestKey7", puremvc_model_new);
    struct Model *model2 = puremvc_model_getInstance("ModelTestKey8", puremvc_model_new);

    model1->registerProxy(model1, puremvc_proxy_new("colors", (char *[]) {"red", "green", "blue", NULL}));
    model2->registerProxy(model2, puremvc_proxy_new("aces", (char *[]) {"clubs", "spades", "hearts", "diamonds", NULL}));

    assert(model1->hasProxy(model1, "colors"));
    assert(!model2->hasProxy(model2, "colors"));

    assert(!model1->hasProxy(model1, "aces"));
    assert(model2->hasProxy(model2, "aces"));

    struct Proxy *colors = model1->removeProxy(model1, "colors");
    if (colors != NULL) puremvc_proxy_free(&colors);
    struct Proxy *aces = model2->removeProxy(model2, "aces");
    if (aces != NULL) puremvc_proxy_free(&aces);

    puremvc_model_removeModel("ModelTestKey7");
    puremvc_model_removeModel("ModelTestKey8");
}
