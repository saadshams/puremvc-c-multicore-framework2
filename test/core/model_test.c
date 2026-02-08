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
    struct ModelMap *modelMap[] = { &(struct ModelMap) { // supply empty key modelMap
        .model = (struct IModel *) &(struct Model){ .multitonKey = "", .proxyMap = (struct ProxyMap *[]) { NULL }, }
    }, NULL };

    // Test Factory Method
    const struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey1");
    const struct Model *concrete = (struct Model *) modelMap[0]->model;

    // test assertions
    assert(model != NULL);
    assert(model == puremvc_model_getInstance(modelMap, "ModelTestKey1"));
    assert(strcmp(modelMap[0]->key, "ModelTestKey1") == 0);
    assert(strcmp(concrete->multitonKey, "ModelTestKey1") == 0);

    assert(puremvc_model_getInstance(NULL, "ModelTestKey1") == model); // pre-initialized

    struct IModel *removedModel = NULL;
    assert(puremvc_model_removeModel("ModelTestKey1", &removedModel) == true);;
    assert(strcmp(((struct Model *) removedModel)->multitonKey, "ModelTestKey1") == 0);
    assert(modelMap[0]->key[0] == '\0');
    model = NULL;
}

void testRegisterAndRetrieveProxy() {
    struct ModelMap *modelMap[] = { &(struct ModelMap) { // supply empty key modelMap with one slot for proxy
        .model = (struct IModel *) &(struct Model) {
            .proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = (struct IProxy *) &(struct Proxy){0} }, NULL }
        }
    }, NULL };

    // register a proxy and retrieve it.
    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey2");
    const struct Model *concrete = (struct Model *) modelMap[0]->model;

    const char **colors = (const char *[]) {"red", "green", "blue", NULL};
    model->registerProxy(model, puremvc_proxy_init, "colors", colors);
    assert(strcmp(concrete->proxyMap[0]->key, "colors") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "colors") == 0);

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
    assert(concrete->proxyMap[0]->key[0] == '\0');
    assert(strcmp(removedProxy->getName(removedProxy), "colors") == 0);

    model->retrieveProxy(model, "colors");

    assert(puremvc_model_removeModel("ModelTestKey2", NULL) == true);
    model = NULL;
}

void testHasProxy() {
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = (struct IModel *) &(struct Model) { .proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = (struct IProxy *) &(struct Proxy){0} }, NULL } }
    }, NULL };

    // Get the Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey3");
    const struct Model *concrete = (struct Model *) modelMap[0]->model;

    // Create and register the test mediator
    const char **aces = (const char *[]) {"clubs", "spades", "hearts", "diamonds", NULL};
    model->registerProxy(model, puremvc_proxy_init, "aces", aces);
    assert(strcmp(concrete->proxyMap[0]->key, "aces") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "aces") == 0);

    // assert that the model.hasProxy method returns true
    // for that proxy name
    assert(model->hasProxy(model, "aces") == true);

    // remove the proxy
    struct IProxy *removedProxy = NULL;
    model->removeProxy(model, "aces", &removedProxy);
    assert(concrete->proxyMap[0]->key[0] == '\0');
    assert(strcmp(removedProxy->getName(removedProxy), "aces") == 0);

    // assert that the model.hasProxy method returns false
    // for that proxy name
    assert(model->hasProxy(model, "aces") == false);

    assert(puremvc_model_removeModel("ModelTestKey3", NULL) == true);
    model = NULL;
}

void testRegisterAndRemoveProxy() {
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = (struct IModel *) &(struct Model) { .proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = (struct IProxy *) &(struct Proxy){0} }, NULL } }
    }, NULL };

    // Get the Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey4");
    const struct Model *concrete = (struct Model *) modelMap[0]->model;

    int *sizes = (int []) {1, 2, 3, 0};

    // Register the proxy
    model->registerProxy(model, puremvc_proxy_init, "sizes", sizes);
    assert(strcmp(concrete->proxyMap[0]->key, "sizes") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "sizes") == 0);

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
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = (struct IModel *) &(struct Model) { .proxyMap = (struct ProxyMap *[]) { &(struct ProxyMap){ .proxy = (struct IProxy *) &(struct Proxy){0} }, NULL } }
    }, NULL };

    // Get a Multiton Model instance
    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey5");
    const struct Model *concrete = (struct Model *) modelMap[0]->model;

    // Create and register the test proxy
    model->registerProxy(model, model_test_proxy, "ModelTestProxy", NULL);
    assert(strcmp(concrete->proxyMap[0]->key, "ModelTestProxy") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "ModelTestProxy") == 0);

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
    struct ModelMap *modelMap[] = { &(struct ModelMap) {
        .model = (struct IModel *) &(struct Model) {}
    }, NULL };

    // Get a Multiton Model instance
    puremvc_model_getInstance(modelMap, "ModelTestKey6");
    const struct Model *concrete = (struct Model *) modelMap[0]->model;

    assert(strcmp(modelMap[0]->key, "ModelTestKey6") == 0);
    assert(strcmp(concrete->multitonKey, "ModelTestKey6") == 0);

    // remove the model
    assert(puremvc_model_removeModel("ModelTestKey6", NULL) == true);
    assert(modelMap[0]->key[0] == '\0');

    // re-create the model without throwing an exception
    puremvc_model_getInstance(modelMap, "ModelTestKey6");
    assert(strcmp(modelMap[0]->key, "ModelTestKey6") == 0);
    assert(strcmp(concrete->multitonKey, "ModelTestKey6") == 0);

    // try removing again
    assert(puremvc_model_removeModel("ModelTestKey6", NULL) == true);;
    assert(modelMap[0]->key[0] == '\0');
}

void testRegisterAndReplaceProxy() {
    struct ModelMap *modelMap[] = { &(struct ModelMap) { // empty model key modelMap with two slots for proxy
        .model = (struct IModel *) &(struct Model) {
            .proxyMap = (struct ProxyMap *[]) {
                &(struct ProxyMap) { .proxy = (struct IProxy *) &(struct Proxy){0} },
                &(struct ProxyMap) { .proxy = (struct IProxy *) &(struct Proxy){0} },
                NULL
            }
        }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey9");

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

void testRegisterAndRemoveMultipleProxies() {
    struct ModelMap *modelMap[] = { &(struct ModelMap) { // empty key model modelMap with four slots for proxy
        .model = (struct IModel *) &(struct Model) {
            .proxyMap = (struct ProxyMap *[]) {
                &(struct ProxyMap) { .proxy = (struct IProxy *) &(struct Proxy){0} }, &(struct ProxyMap){ .proxy = (struct IProxy *) &(struct Proxy){0} },
                &(struct ProxyMap) { .proxy = (struct IProxy *) &(struct Proxy){0} }, &(struct ProxyMap){ .proxy = (struct IProxy *) &(struct Proxy){0} },
                NULL
            }
        }
    }, NULL };

    struct IModel *model = puremvc_model_getInstance(modelMap, "ModelTestKey10");
    const struct Model *concrete = (struct Model *) modelMap[0]->model;

    // Register four proxies and verify that each is correctly associated to their dictionaries
    model->registerProxy(model, puremvc_proxy_init, "proxy0", NULL);
    assert(strcmp(concrete->proxyMap[0]->key, "proxy0") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "proxy0") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy1", NULL);
    assert(strcmp(concrete->proxyMap[1]->key, "proxy1") == 0);
    assert(strcmp(concrete->proxyMap[1]->proxy->getName(concrete->proxyMap[1]->proxy), "proxy1") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy2", NULL);
    assert(strcmp(concrete->proxyMap[2]->key, "proxy2") == 0);
    assert(strcmp(concrete->proxyMap[2]->proxy->getName(concrete->proxyMap[2]->proxy), "proxy2") == 0);

    model->registerProxy(model, puremvc_proxy_init, "proxy3", NULL);
    assert(strcmp(concrete->proxyMap[3]->key, "proxy3") == 0);
    assert(strcmp(concrete->proxyMap[3]->proxy->getName(concrete->proxyMap[3]->proxy), "proxy3") == 0);

    // Remove the second proxy (middle) and verify that remaining mediators 0, 2, 3 are shifted correctly
    struct IProxy *removedProxy1 = NULL;
    assert(model->removeProxy(model, "proxy1", &removedProxy1) == true);
    assert(strcmp(concrete->proxyMap[0]->key, "proxy0") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "proxy0") == 0);
    assert(strcmp(concrete->proxyMap[1]->key, "proxy2") == 0);
    assert(strcmp(concrete->proxyMap[1]->proxy->getName(concrete->proxyMap[1]->proxy), "proxy2") == 0);
    assert(strcmp(concrete->proxyMap[2]->key, "proxy3") == 0);
    assert(strcmp(concrete->proxyMap[2]->proxy->getName(concrete->proxyMap[2]->proxy), "proxy3") == 0);

    // Remove the last proxy and verify the remaining 0, 2 stay in place
    struct IProxy *removedProxy3 = NULL;
    assert(model->removeProxy(model, "proxy3", &removedProxy3) == true);;
    assert(strcmp(concrete->proxyMap[0]->key, "proxy0") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "proxy0") == 0);
    assert(strcmp(concrete->proxyMap[1]->key, "proxy2") == 0);
    assert(strcmp(concrete->proxyMap[1]->proxy->getName(concrete->proxyMap[1]->proxy), "proxy2") == 0);

    // Remove the first mediator and verify that subsequent mediator 2 shift left
    struct IProxy *removedProxy0 = NULL;
    assert(model->removeProxy(model, "proxy0", &removedProxy0) == true);;
    assert(strcmp(concrete->proxyMap[0]->key, "proxy2") == 0);
    assert(strcmp(concrete->proxyMap[0]->proxy->getName(concrete->proxyMap[0]->proxy), "proxy2") == 0);

    // Remove the remaining mediator3 and confirm that the dictionary is NULL
    struct IProxy *removedProxy2 = NULL;
    assert(model->removeProxy(model, "proxy2", &removedProxy2) == true);;
    assert(concrete->proxyMap[0]->key[0] == '\0'); // proxyMap is empty
    assert(concrete->proxyMap[1]->key[0] == '\0');
    assert(concrete->proxyMap[2]->key[0] == '\0');
    assert(concrete->proxyMap[3]->key[0] == '\0');

    assert(puremvc_model_removeModel("ModelTestKey10", NULL) == true);;
    model = NULL;
}

void TestModelShiftLeft() {
    struct ModelMap *modelMap[] = { // empty key model modelMap with 4 slots
        &(struct ModelMap) { .model = (struct IModel *) &(struct Model){} },
        &(struct ModelMap) { .model = (struct IModel *) &(struct Model){} },
        &(struct ModelMap) { .model = (struct IModel *) &(struct Model){} },
        &(struct ModelMap) { .model = (struct IModel *) &(struct Model){} },
        NULL
    };

    // create 4 instances
    puremvc_model_getInstance(modelMap, "model0");
    assert(strcmp(modelMap[0]->key, "model0") == 0);
    assert(strcmp(((struct Model *) modelMap[0]->model)->multitonKey, "model0") == 0);

    puremvc_model_getInstance(modelMap, "model1");
    assert(strcmp(modelMap[1]->key, "model1") == 0);
    assert(strcmp(((struct Model *) modelMap[1]->model)->multitonKey, "model1") == 0);

    puremvc_model_getInstance(modelMap, "model2");
    assert(strcmp(modelMap[2]->key, "model2") == 0);
    assert(strcmp(((struct Model *) modelMap[2]->model)->multitonKey, "model2") == 0);

    puremvc_model_getInstance(modelMap, "model3");
    assert(strcmp(modelMap[3]->key, "model3") == 0);
    assert(strcmp(((struct Model *) modelMap[3]->model)->multitonKey, "model3") == 0);

    // remove
    struct IModel *model1 = NULL;
    assert(puremvc_model_removeModel("model1", &model1) == true); // remove middle, remaining 0, 2, 3
    assert(strcmp(((struct Model *) model1)->multitonKey, "model1") == 0);
    assert(strcmp(modelMap[0]->key, "model0") == 0);
    assert(strcmp(((struct Model *) modelMap[0]->model)->multitonKey, "model0") == 0);
    assert(strcmp(modelMap[1]->key, "model2") == 0);
    assert(strcmp(((struct Model *) modelMap[1]->model)->multitonKey, "model2") == 0);
    assert(strcmp(modelMap[2]->key, "model3") == 0);
    assert(strcmp(((struct Model *) modelMap[2]->model)->multitonKey, "model3") == 0);
    assert(modelMap[3]->key[0] == '\0');

    struct IModel *model3 = NULL;
    assert(puremvc_model_removeModel("model3", &model3) == true); // remove last, remaining 0, 2
    assert(strcmp(((struct Model *) model3)->multitonKey, "model3") == 0);
    assert(strcmp(modelMap[0]->key, "model0") == 0);
    assert(strcmp(((struct Model *) modelMap[0]->model)->multitonKey, "model0") == 0);
    assert(strcmp(modelMap[1]->key, "model2") == 0);
    assert(strcmp(((struct Model *) modelMap[1]->model)->multitonKey, "model2") == 0);
    assert(modelMap[2]->key[0] == '\0');
    assert(modelMap[3]->key[0] == '\0');

    struct IModel *model0 = NULL;
    assert(puremvc_model_removeModel("model0", &model0) == true); // remove first, remaining 2
    assert(strcmp(((struct Model *) model0)->multitonKey, "model0") == 0);
    assert(strcmp(modelMap[0]->key, "model2") == 0);
    assert(strcmp(((struct Model *) modelMap[0]->model)->multitonKey, "model2") == 0);
    assert(modelMap[1]->key[0] == '\0');
    assert(modelMap[2]->key[0] == '\0');
    assert(modelMap[3]->key[0] == '\0');

    struct IModel *model2 = NULL;
    assert(puremvc_model_removeModel("model2", &model2) == true); // remove remaining
    assert(strcmp(((struct Model *) model2)->multitonKey, "model2") == 0);
    assert(modelMap[0]->key[0] == '\0');
    assert(modelMap[1]->key[0] == '\0');
    assert(modelMap[2]->key[0] == '\0');
    assert(modelMap[3]->key[0] == '\0');
}
